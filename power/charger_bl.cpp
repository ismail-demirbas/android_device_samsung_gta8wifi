#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define BL_PATH "/sys/class/backlight/sprd_backlight/brightness"
#define BL_LEVEL "128"
#define MAX_DEV 8
#define KEYBYTES (KEY_MAX / 8 + 1)

static int open_keys(struct pollfd *p) {
    int n = 0;
    DIR *d = opendir("/dev/input");
    if (!d) return 0;
    struct dirent *e;
    while (n < MAX_DEV && (e = readdir(d))) {
        if (strncmp(e->d_name, "event", 5)) continue;
        char path[64];
        snprintf(path, sizeof(path), "/dev/input/%s", e->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) continue;
        unsigned char bits[KEYBYTES];
        memset(bits, 0, sizeof(bits));
        if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bits)), bits) >= 0 &&
            (bits[KEY_POWER / 8] & (1 << (KEY_POWER % 8)))) {
            p[n].fd = fd;
            p[n].events = POLLIN;
            n++;
        } else {
            close(fd);
        }
    }
    closedir(d);
    return n;
}

static void backlight_on(void) {
    for (int i = 0; i < 20; i++) {
        int fd = open(BL_PATH, O_WRONLY | O_CLOEXEC);
        if (fd < 0) return;
        ssize_t r = write(fd, BL_LEVEL, 3);
        (void)r;
        close(fd);
        usleep(150000);
    }
}

int main(void) {
    struct pollfd p[MAX_DEV];
    int n = 0;
    for (int i = 0; i < 60 && !(n = open_keys(p)); i++) sleep(1);
    if (!n) return 1;
    while (poll(p, n, -1) >= 0) {
        for (int i = 0; i < n; i++) {
            if (!(p[i].revents & POLLIN)) continue;
            struct input_event ev;
            while (read(p[i].fd, &ev, sizeof(ev)) == (ssize_t)sizeof(ev))
                if (ev.type == EV_KEY && ev.code == KEY_POWER && ev.value == 1) {
                    backlight_on();
                }
        }
    }
    return 1;
}
