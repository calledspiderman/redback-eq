#ifndef RADIOFORM_EQ_WINDOW_H
#define RADIOFORM_EQ_WINDOW_H

#include <gtk/gtk.h>
#include <stdbool.h>
#include "radioform_types.h"

typedef struct EqWindow EqWindow;

typedef void (*eq_band_cb)(void* userdata, int band, float gain_db);
typedef void (*eq_preamp_cb)(void* userdata, float preamp_db);
typedef void (*eq_bypass_cb)(void* userdata, bool bypass);
typedef void (*eq_preset_cb)(void* userdata, const radioform_preset_t* preset);

EqWindow* eq_window_new(void);
void eq_window_destroy(EqWindow* win);
void eq_window_present(EqWindow* win);
void eq_window_load_preset(EqWindow* win, const radioform_preset_t* preset);
void eq_window_set_callbacks(EqWindow* win,
    eq_band_cb on_band, void* band_userdata,
    eq_preamp_cb on_preamp, void* preamp_userdata,
    eq_bypass_cb on_bypass, void* bypass_userdata,
    eq_preset_cb on_preset, void* preset_userdata);

#endif
