#include <ctype.h>
#include <cutils/log.h>
#include <cutils/properties.h>
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

#define BUF_LENGTH                      (512)

int format_device(const char* devicePath, const char* label)
{
    char sector[BUF_LENGTH];
    int fd;
    pid_t child;
    int status;

    fd = open(devicePath, O_RDONLY);
    if (fd <= 0) {
        ERROR("open device error: %s", strerror(errno));
        return 1;
    }
    memset(sector, 0, BUF_LENGTH);
    read(fd, sector, BUF_LENGTH);
    close(fd);
    if ((sector[510] == 0x55) && (sector[511] == 0xaa)) {
        INFO("Don't need to format %s\n", devicePath);
        property_set("sys.format_device", label);
        return 0;
    } else {
        INFO("Start format %s\n", devicePath);
        child = fork();
        if (child == 0) {
            INFO("fork to format %s", devicePath);
            execl("/system/bin/newfs_msdos", "/system/bin/newfs_msdos",
                    "-F", "32", "-O", "android", "-c", "8", "-L", label, devicePath, NULL);
            exit(0);
        } else {
            DEBUG("wait for format %s", devicePath);
            while (waitpid(-1, &status, 0) != child);
            INFO("format %s ok", devicePath);
        }
    }
    property_set("sys.format_device", label);
    return 0;
}

int main(int nargs, char **args) {
    format_device(args[1], args[2]);
    return 0;
}
