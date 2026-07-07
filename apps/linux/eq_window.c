#include "eq_window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NUM_BANDS 10

static const float DEFAULT_FREQUENCIES[NUM_BANDS] = {
    32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000
};

struct EqWindow {
    GtkWidget* window;
    GtkWidget* scales[NUM_BANDS + 1];
    GtkWidget* bypass_switch;
    GtkWidget* freq_labels[NUM_BANDS + 1];

    radioform_preset_t current;

    eq_band_cb on_band;
    void* band_userdata;
    eq_preamp_cb on_preamp;
    void* preamp_userdata;
    eq_bypass_cb on_bypass;
    void* bypass_userdata;
    eq_preset_cb on_preset;
    void* preset_userdata;

    int updating;
};

static void format_frequency(char* buf, size_t size, float hz) {
    if (hz < 1000)
        snprintf(buf, size, "%.0f", hz);
    else if (hz < 10000)
        snprintf(buf, size, "%.1fK", hz / 1000);
    else
        snprintf(buf, size, "%.0fK", hz / 1000);
}

static void on_scale_changed(GtkRange* range, gpointer userdata) {
    EqWindow* win = (EqWindow*)userdata;
    if (win->updating) return;

    int index = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(range), "band-index"));

    float value = (float)gtk_range_get_value(range);

    if (index < NUM_BANDS) {
        win->current.bands[index].gain_db = value;
        if (win->on_band)
            win->on_band(win->band_userdata, index, value);
    } else {
        win->current.preamp_db = value;
        if (win->on_preamp)
            win->on_preamp(win->preamp_userdata, value);
    }
}

static void on_bypass_toggled(GtkToggleButton* btn, gpointer userdata) {
    EqWindow* win = (EqWindow*)userdata;
    bool bypass = !gtk_switch_get_active(GTK_SWITCH(btn));
    if (win->on_bypass)
        win->on_bypass(win->bypass_userdata, bypass);
}

EqWindow* eq_window_new(void) {
    EqWindow* win = calloc(1, sizeof(EqWindow));

    win->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win->window), "Radioform EQ");
    gtk_window_set_default_size(GTK_WINDOW(win->window), 400, 300);
    gtk_window_set_resizable(GTK_WINDOW(win->window), FALSE);
    g_signal_connect(win->window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(win->window), vbox);

    // Header: bypass toggle + preamp
    GtkWidget* header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(vbox), header, FALSE, FALSE, 4);

    GtkWidget* logo = gtk_label_new("Radioform");
    gtk_widget_set_name(logo, "radioform-logo");
    gtk_box_pack_start(GTK_BOX(header), logo, FALSE, FALSE, 0);

    gtk_box_pack_end(GTK_BOX(header), gtk_label_new(""), TRUE, TRUE, 0);

    win->bypass_switch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(win->bypass_switch), TRUE);
    g_signal_connect(win->bypass_switch, "notify::active", G_CALLBACK(on_bypass_toggled), win);
    gtk_box_pack_end(GTK_BOX(header), win->bypass_switch, FALSE, FALSE, 0);

    // EQ sliders
    GtkWidget* grid = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 4);

    int display_order[] = {10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < NUM_BANDS + 1; i++) {
        int idx = display_order[i];
        GtkWidget* band_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_box_pack_start(GTK_BOX(grid), band_box, TRUE, TRUE, 0);

        GtkWidget* scale = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, -12.0, 12.0, 0.5);
        gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);
        gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_TOP);
        gtk_widget_set_size_request(scale, -1, 160);
        gtk_range_set_inverted(GTK_RANGE(scale), TRUE);

        g_object_set_data(G_OBJECT(scale), "band-index", GINT_TO_POINTER(idx));
        g_signal_connect(scale, "value-changed", G_CALLBACK(on_scale_changed), win);

        if (idx == 10) {
            gtk_range_set_value(GTK_RANGE(scale), 0.0);
        } else {
            gtk_range_set_value(GTK_RANGE(scale), 0.0);
            win->current.bands[idx].frequency_hz = DEFAULT_FREQUENCIES[idx];
            win->current.bands[idx].enabled = false;
            win->current.bands[idx].type = RADIOFORM_FILTER_PEAK;
            win->current.bands[idx].q_factor = 1.0;
        }

        win->scales[idx] = scale;
        gtk_box_pack_start(GTK_BOX(band_box), scale, TRUE, TRUE, 0);

        char label[16];
        format_frequency(label, sizeof(label),
            idx < NUM_BANDS ? DEFAULT_FREQUENCIES[idx] : 0);
        if (idx == 10) snprintf(label, sizeof(label), "Pre");

        win->freq_labels[idx] = gtk_label_new(label);
        gtk_box_pack_start(GTK_BOX(band_box), win->freq_labels[idx], FALSE, FALSE, 0);
    }

    // Preset row at bottom
    GtkWidget* preset_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), preset_box, FALSE, FALSE, 4);

    GtkWidget* preset_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(preset_combo), "Flat");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(preset_combo), "Bass Boost");
    gtk_widget_set_hexpand(preset_combo, TRUE);
    gtk_box_pack_start(GTK_BOX(preset_box), preset_combo, TRUE, TRUE, 0);

    GtkWidget* save_btn = gtk_button_new_with_label("Save");
    gtk_box_pack_start(GTK_BOX(preset_box), save_btn, FALSE, FALSE, 0);

    gtk_widget_show_all(win->window);
    gtk_window_set_keep_above(GTK_WINDOW(win->window), TRUE);

    // CSS
    GtkCssProvider* css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css,
        "#radioform-logo { font-size: 14px; font-weight: bold; }\n"
        "scale slider { min-width: 16px; min-height: 16px; }\n"
        "scale trough { min-height: 140px; }\n", -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gtk_window_get_screen(GTK_WINDOW(win->window)),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    return win;
}

void eq_window_destroy(EqWindow* win) {
    if (win) {
        gtk_widget_destroy(win->window);
        free(win);
    }
}

void eq_window_present(EqWindow* win) {
    if (win && win->window) {
        gtk_window_present(GTK_WINDOW(win->window));
    }
}

void eq_window_load_preset(EqWindow* win, const radioform_preset_t* preset) {
    if (!win || !preset) return;
    win->updating = 1;
    memcpy(&win->current, preset, sizeof(radioform_preset_t));

    for (int i = 0; i < (int)preset->num_bands && i < NUM_BANDS; i++) {
        gtk_range_set_value(GTK_RANGE(win->scales[i]), preset->bands[i].gain_db);
        win->current.bands[i] = preset->bands[i];
    }

    gtk_range_set_value(GTK_RANGE(win->scales[10]), preset->preamp_db);
    win->current.preamp_db = preset->preamp_db;
    win->current.limiter_enabled = preset->limiter_enabled;
    win->current.limiter_threshold_db = preset->limiter_threshold_db;
    strncpy(win->current.name, preset->name, sizeof(win->current.name) - 1);

    win->updating = 0;
}

void eq_window_set_callbacks(EqWindow* win,
    eq_band_cb on_band, void* band_userdata,
    eq_preamp_cb on_preamp, void* preamp_userdata,
    eq_bypass_cb on_bypass, void* bypass_userdata,
    eq_preset_cb on_preset, void* preset_userdata)
{
    win->on_band = on_band;
    win->band_userdata = band_userdata;
    win->on_preamp = on_preamp;
    win->preamp_userdata = preamp_userdata;
    win->on_bypass = on_bypass;
    win->bypass_userdata = bypass_userdata;
    win->on_preset = on_preset;
    win->preset_userdata = preset_userdata;
}
