#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <cutils/log.h>

// Unisoc Hall switch code from the kernel input UAPI.
static constexpr unsigned int HALL_SW_FLIP = 0x15;

static int openHall() {
    char path[64], name[64];
    for (int i = 0; i < 32; i++) {
        snprintf(path, sizeof(path), "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0) continue;
        memset(name, 0, sizeof(name));
        ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name);
        if (!strcmp(name, "hall_irq")) return fd;
        close(fd);
    }
    return -1;
}

static int emit(int ufd, int type, int code, int value) {
    struct input_event e;
    memset(&e, 0, sizeof(e));
    e.type = type; e.code = code; e.value = value;
    return TEMP_FAILURE_RETRY(write(ufd, &e, sizeof(e)));
}

int main(void) {
    int ufd = TEMP_FAILURE_RETRY(open("/dev/uinput", O_WRONLY | O_NONBLOCK));
    if (ufd < 0) { ALOGE("uinput open: %s", strerror(errno)); return 1; }
    if (ioctl(ufd, UI_SET_EVBIT, EV_SW) || ioctl(ufd, UI_SET_EVBIT, EV_SYN) ||
        ioctl(ufd, UI_SET_SWBIT, SW_LID)) {
        ALOGE("uinput bits: %s", strerror(errno)); return 1;
    }
    struct uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-folio");
    uidev.id.bustype = BUS_VIRTUAL;
    if (write(ufd, &uidev, sizeof(uidev)) < 0 || ioctl(ufd, UI_DEV_CREATE) < 0) {
        ALOGE("uinput create: %s", strerror(errno)); return 1;
    }

    int hfd = -1;
    for (int i = 0; i < 120 && (hfd = openHall()) < 0; i++) sleep(5);
    if (hfd < 0) { ALOGE("hall_irq not found"); return 1; }

    unsigned long sw = 0;
    if (ioctl(hfd, EVIOCGSW(sizeof(sw)), &sw) >= 0) {
        emit(ufd, EV_SW, SW_LID, (sw >> HALL_SW_FLIP) & 1);
        emit(ufd, EV_SYN, SYN_REPORT, 0);
    }
    ALOGI("Polling hall_irq");

    struct pollfd p = { hfd, POLLIN, 0 };
    struct input_event ev;
    while (poll(&p, 1, -1) > 0) {
        while (read(hfd, &ev, sizeof(ev)) == (ssize_t)sizeof(ev)) {
            if (ev.type == EV_SW && ev.code == HALL_SW_FLIP) {
                emit(ufd, EV_SW, SW_LID, ev.value);
                emit(ufd, EV_SYN, SYN_REPORT, 0);
                ALOGI("lid %s", ev.value ? "closed" : "open");
            }
        }
    }
    close(hfd);
    close(ufd);
    return 1;
}
