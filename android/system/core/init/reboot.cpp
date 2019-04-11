
#include <fcntl.h>
#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <sched.h>

#include <selinux/selinux.h>
#include <selinux/label.h>

#include <sys/ioctl.h>

#include <fs_mgr.h>
#include <android-base/file.h>
#include <android-base/parseint.h>
#include <android-base/stringprintf.h>
#include <bootloader_message/bootloader_message.h>
#include <cutils/android_reboot.h>
#include <logwrap/logwrap.h>
#include <private/android_filesystem_config.h>


#include "log.h"
#include "property_service.h"
#include "service.h"
#include "util.h"
#include "reboot.h"

#if defined LIGHT
    #include <hardware/drv_display.h>
#elif defined LIGHT2 || LIGHT3
    #include <hardware/sunxi_display2.h>
#endif

//#define chmod DO_NOT_USE_CHMOD_USE_FCHMODAT_SYMLINK_NOFOLLOW

#define UNMOUNT_CHECK_MS 5000
#define UNMOUNT_CHECK_TIMES 10
static const int kTerminateServiceDelayMicroSeconds = 50000;



// Turn off backlight while we are performing power down cleanup activities.
void turnOffBacklight1() {
    static const char off[] = "0";

    android::base::WriteStringToFile(off, "/sys/class/leds/lcd-backlight/brightness");

    static const char backlightDir[] = "/sys/class/backlight";
    std::unique_ptr<DIR, int(*)(DIR*)> dir(opendir(backlightDir), closedir);
    if (!dir) {
        return;
    }

    struct dirent *dp;
    while ((dp = readdir(dir.get())) != NULL) {
        if (((dp->d_type != DT_DIR) && (dp->d_type != DT_LNK)) ||
                (dp->d_name[0] == '.')) {
            continue;
        }

        std::string fileName = android::base::StringPrintf("%s/%s/brightness",
                                                           backlightDir,
                                                           dp->d_name);
        android::base::WriteStringToFile(off, fileName);
    }
}



static void turnOffBacklight() {
    int fd = 0;
    fd = open("/dev/disp", O_RDONLY);
    if (fd < 0) {
        ERROR("powerctl:open /dev/disp ERROR");
        return;
    }

    int brightness = 0;
    int i = 0;
    unsigned long  args[3];
    int err = 0;
    for (i = 0; i < 2; i++) {
        args[0] = i;
        #if defined LIGHT
            if (ioctl(fd,DISP_CMD_GET_OUTPUT_TYPE,args) == DISP_OUTPUT_TYPE_LCD) {
                args[1]  = brightness;
                args[2]  = 0;
                err = ioctl(fd,DISP_CMD_LCD_SET_BRIGHTNESS,args);
                INFO("powerctl:ioctl turn off backlight,err=%d",err);

                break;
            }
        #elif defined LIGHT2 || LIGHT3
            if (ioctl(fd,DISP_GET_OUTPUT_TYPE,args) == DISP_OUTPUT_TYPE_LCD) {
                args[1]  = brightness;
                args[2]  = 0;
                err = ioctl(fd,DISP_LCD_SET_BRIGHTNESS,args);
                INFO("powerctl:ioctl turn off backlight,err=%d",err);

                break;
            }
        #endif
    }

    if (fd != 0) {
        close(fd);
    }

}




static void unmount_and_fsck(const struct mntent *entry) {
    if (strcmp(entry->mnt_type, "f2fs") && strcmp(entry->mnt_type, "ext4"))
        return;

    /* First, lazily unmount the directory. This unmount request finishes when
     * all processes that open a file or directory in |entry->mnt_dir| exit.
     */
    TEMP_FAILURE_RETRY(umount2(entry->mnt_dir, MNT_DETACH));

    /* Next, kill all processes except init, kthreadd, and kthreadd's
     * children to finish the lazy unmount. Killing all processes here is okay
     * because this callback function is only called right before reboot().
     * It might be cleaner to selectively kill processes that actually use
     * |entry->mnt_dir| rather than killing all, probably by reusing a function
     * like killProcessesWithOpenFiles() in vold/, but the selinux policy does
     * not allow init to scan /proc/<pid> files which the utility function
     * heavily relies on. The policy does not allow the process to execute
     * killall/pkill binaries either. Note that some processes might
     * automatically restart after kill(), but that is not really a problem
     * because |entry->mnt_dir| is no longer visible to such new processes.
     */
    ServiceManager::GetInstance().ForEachService([] (Service* s) { s->Stop(); });
    TEMP_FAILURE_RETRY(kill(-1, SIGKILL));

    // Restart Watchdogd to allow us to complete umounting and fsck
    Service *svc = ServiceManager::GetInstance().FindServiceByName("watchdogd");
    if (svc) {
        do {
            sched_yield(); // do not be so eager, let cleanup have priority
            ServiceManager::GetInstance().ReapAnyOutstandingChildren();
        } while (svc->flags() & SVC_RUNNING); // Paranoid Cargo
        svc->Start();
    }

    turnOffBacklight();

    int count = 0;
    while (count++ < UNMOUNT_CHECK_TIMES) {
        int fd = TEMP_FAILURE_RETRY(open(entry->mnt_fsname, O_RDONLY | O_EXCL));
        if (fd >= 0) {
            /* |entry->mnt_dir| has sucessfully been unmounted. */
            close(fd);
            break;
        } else if (errno == EBUSY) {
            /* Some processes using |entry->mnt_dir| are still alive. Wait for a
             * while then retry.
             */
            TEMP_FAILURE_RETRY(
                usleep(UNMOUNT_CHECK_MS * 1000 / UNMOUNT_CHECK_TIMES));
            continue;
        } else {
            /* Cannot open the device. Give up. */
            return;
        }
    }

    // NB: With watchdog still running, there is no cap on the time it takes
    // to complete the fsck, from the users perspective the device graphics
    // and responses are locked-up and they may choose to hold the power
    // button in frustration if it drags out.

    int st;
    if (!strcmp(entry->mnt_type, "f2fs")) {
        const char *f2fs_argv[] = {
            "/system/bin/fsck.f2fs", "-f", entry->mnt_fsname,
        };
        android_fork_execvp_ext(ARRAY_SIZE(f2fs_argv), (char **)f2fs_argv,
                                &st, true, LOG_KLOG, true, NULL, NULL, 0);
    } else if (!strcmp(entry->mnt_type, "ext4")) {
        const char *ext4_argv[] = {
            "/system/bin/e2fsck", "-f", "-y", entry->mnt_fsname,
        };
        android_fork_execvp_ext(ARRAY_SIZE(ext4_argv), (char **)ext4_argv,
                                &st, true, LOG_KLOG, true, NULL, NULL, 0);
    }
}




int handlePowerctl(const std::vector<std::string>& args) {
    const char* command = args[0].c_str();
    int len = 0;
    unsigned int cmd = 0;
    const char *reboot_target = "";
    void (*callback_on_ro_remount)(const struct mntent*) = NULL;

    if (strncmp(command, "shutdown", 8) == 0) {
        cmd = ANDROID_RB_POWEROFF;
        len = 8;
    } else if (strncmp(command, "reboot", 6) == 0) {
        cmd = ANDROID_RB_RESTART2;
        len = 6;
    } else {
        ERROR("powerctl: unrecognized command '%s'\n", command);
        return -EINVAL;
    }

    if (command[len] == ',') {
        if (cmd == ANDROID_RB_POWEROFF &&
            !strcmp(&command[len + 1], "userrequested")) {
            // The shutdown reason is PowerManager.SHUTDOWN_USER_REQUESTED.
            // Run fsck once the file system is remounted in read-only mode.
            callback_on_ro_remount = unmount_and_fsck;
        } else if (cmd == ANDROID_RB_RESTART2) {
            reboot_target = &command[len + 1];
        }
    } else if (command[len] != '\0') {
        ERROR("powerctl: unrecognized reboot target '%s'\n", &command[len]);
        return -EINVAL;
    }

    std::string timeout = property_get("ro.build.shutdown_timeout");
    unsigned int delay = 0;

    if (android::base::ParseUint(timeout.c_str(), &delay) && delay > 0) {
        Timer t;
        // Ask all services to terminate.
        ServiceManager::GetInstance().ForEachService(
            [] (Service* s) { s->Terminate(); });

        while (t.duration() < delay) {
            ServiceManager::GetInstance().ReapAnyOutstandingChildren();

            int service_count = 0;
            ServiceManager::GetInstance().ForEachService(
                [&service_count] (Service* s) {
                    // Count the number of services running.
                    // Exclude the console as it will ignore the SIGTERM signal
                    // and not exit.
                    // Note: SVC_CONSOLE actually means "requires console" but
                    // it is only used by the shell.
                    if (s->pid() != 0 && (s->flags() & SVC_CONSOLE) == 0) {
                        service_count++;
                    }
                });

            if (service_count == 0) {
                // All terminable services terminated. We can exit early.
                break;
            }

            // Wait a bit before recounting the number or running services.
            usleep(kTerminateServiceDelayMicroSeconds);
        }
        NOTICE("Terminating running services took %.02f seconds", t.duration());
    }

    return android_reboot_with_callback(cmd, 0, reboot_target,
                                        callback_on_ro_remount);
}



