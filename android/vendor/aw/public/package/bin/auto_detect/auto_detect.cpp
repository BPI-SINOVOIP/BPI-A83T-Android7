#include <ctype.h>
#include <cutils/log.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define ERROR(x...)   ALOGE(x)
#define WARNING(x...) ALOGW(x)
#define NOTICE(x...)  ALOGI(x)
#define INFO(x...)    ALOGI(x)
#define DEBUG(x...)   ALOGD(x)
#define VERBOSE(x...) ALOGV(x)

#define DETECT_MODULE                   ("/system/vendor/modules/auto_detect.ko")
#define INSMOD_PATH                     ("/system/vendor/modules/")
#define I2C_DEVICE_CONFIG_PATH          ("/sys/devices/auto_detect/devlist")
#define CACHE_CONFIG_DIR                ("/cache/auto_detect/")
#define CACHE_CONFIG_FILE               ("/cache/auto_detect/devlist")
#define BUF_LENGTH                      (1024)

static int insmod(const char *filename, const char *options) {
    int fd = open(filename, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd == -1) {
        ERROR("insmod: open(\"%s\") failed: %s", filename, strerror(errno));
        return -1;
    }
    int rc = syscall(__NR_finit_module, fd, options, 0);
    if (rc == -1) {
        ERROR("finit_module for \"%s\" failed: %s", filename, strerror(errno));
    }
    close(fd);
    return rc;
}

static int auto_detect_inner() {
    char options[1];
    int ret = 0;
    int buf_size = 0;
    options[0] = '\0';
    char buf[BUF_LENGTH];
    FILE *fp;
    if (access(CACHE_CONFIG_FILE, F_OK) == 0) {
        while ((fp = fopen(CACHE_CONFIG_FILE, "rb")) == NULL) {
            ERROR("can't not open file %s!\n", CACHE_CONFIG_FILE);
            sleep(1);
        }
    } else {
        insmod(DETECT_MODULE, options);
        while ((fp = fopen(I2C_DEVICE_CONFIG_PATH, "rb")) == NULL) {
            ERROR("can't not open file!\n");
            sleep(1);
        }

        while (true) {
            memset(&buf, 0, sizeof(buf));
            fseek(fp , 0 , SEEK_SET);
            buf_size = fread(buf, 1, 1024, fp);
            INFO("[auto_detect]devlist buf: %s\n", buf);
            if (strstr(buf, "no_device") != NULL) {
                NOTICE("[auto_detect]:no device used!\n");
                fclose(fp);
                return 0;
            }

            if (strstr(buf , "end_of_list") == NULL) {
                sleep(1);
            } else {
                mkdir(CACHE_CONFIG_DIR, 0700);
                int fd = open(CACHE_CONFIG_FILE, O_RDWR|O_CREAT|O_TRUNC|O_CLOEXEC, 0600);
                if (fd >= 0) {
                    INFO("[auto_detect]devlist write_buf : %s\n", buf);
                    write(fd, buf, (buf_size + 1));
                    close(fd);
                } else {
                    ERROR("could not open %s\n", CACHE_CONFIG_FILE);
                }
                break;
            }
        }
    }

    memset(&buf, 0, sizeof(buf));
    fseek(fp , 0 , SEEK_SET);
    while (fgets(buf, 128, fp)) {
        char module_name[128] = {'\0'};
        char insmod_name[128];
        char ko[] = ".ko";
        char ch = '\"';
        char * position1 ;
        char * position2 ;
        int s1 = 0, s2 = 0, k = 0;

        memset(&insmod_name, 0, sizeof(insmod_name));
        memset(&module_name, 0, sizeof(module_name));
        position1 = strchr(buf, ch);
        position2 = strrchr(buf, ch);

        if ((position1 != NULL) && (position2 != NULL)) {
            s1 = position1 - buf + 1;
            s2 = position2 - buf;

            while (s1 != s2) {
                module_name[k++] = buf[s1++];
            }
            module_name[k] = '\0';
            INFO("module_name:%s\n", module_name);

            sprintf(insmod_name, "%s%s%s", INSMOD_PATH, module_name, ko);
            INFO("start to insmod %s\n", insmod_name);

            ret = insmod(insmod_name, options);
        }
        memset(&buf,0,sizeof(buf));
    }
    fclose(fp);

    return ret;
}

int auto_detect() {
    pid_t pid;
    pid = fork();
    int ret = 0;
    int child_ret = 0;
    int status;
    if (pid > 0) {
        /* Parent.  Wait for the child to return */
        int wp_ret = TEMP_FAILURE_RETRY(waitpid(pid, &status, 0));
        if (wp_ret < 0) {
            /* Unexpected error code. We will continue anyway. */
            NOTICE("waitpid failed rc=%d: %s\n", wp_ret, strerror(errno));
        }

        if (WIFEXITED(status)) {
            ret = WEXITSTATUS(status);
        } else {
            ret = -1;
        }
    } else if (pid == 0) {
        /* child, call fs_mgr_mount_all() */
        child_ret = auto_detect_inner();
        if (child_ret == -1) {
            ERROR("auto_detect_inner()  returned an error\n");
        }
        _exit(child_ret);
    } else {
        /* fork failed, return an error */
        return -1;
    }
    return ret;
}

int main(/*int argc, char *argv[]*/) {
    auto_detect();
    return 0;
}
