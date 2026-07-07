#include "control_client.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

static int connect_socket(void) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, "/tmp/radioform-control.sock", sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int control_connect(void) {
    return connect_socket();
}

static int send_cmd(int fd, const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return write(fd, buf, strlen(buf));
}

int control_send_bypass(int fd, bool bypass) {
    return send_cmd(fd, "bypass %d\n", bypass ? 1 : 0);
}

int control_send_band_gain(int fd, int index, float value) {
    return send_cmd(fd, "band %d %.1f\n", index, value);
}

int control_send_band_freq(int fd, int index, float value) {
    return send_cmd(fd, "freq %d %.1f\n", index, value);
}

int control_send_band_q(int fd, int index, float value) {
    return send_cmd(fd, "q %d %.2f\n", index, value);
}

int control_send_preamp(int fd, float value) {
    return send_cmd(fd, "preamp %.1f\n", value);
}

int control_send_preset(int fd, const radioform_preset_t* preset) {
    // Send each band individually
    int ret = 0;
    for (int i = 0; i < (int)preset->num_bands; i++) {
        ret += control_send_band_gain(fd, i, preset->bands[i].gain_db);
        ret += control_send_band_freq(fd, i, preset->bands[i].frequency_hz);
        ret += control_send_band_q(fd, i, preset->bands[i].q_factor);
    }
    ret += control_send_preamp(fd, preset->preamp_db);
    return ret;
}
