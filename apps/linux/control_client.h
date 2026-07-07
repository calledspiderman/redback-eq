#ifndef RADIOFORM_CONTROL_CLIENT_H
#define RADIOFORM_CONTROL_CLIENT_H

#include <stdbool.h>
#include "radioform_types.h"

int control_connect(void);
int control_send_bypass(int fd, bool bypass);
int control_send_band_gain(int fd, int index, float value);
int control_send_band_freq(int fd, int index, float value);
int control_send_band_q(int fd, int index, float value);
int control_send_preamp(int fd, float value);
int control_send_preset(int fd, const radioform_preset_t* preset);

#endif
