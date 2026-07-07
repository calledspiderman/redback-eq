#include <gtk/gtk.h>
#include <libayatana-appindicator/app-indicator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "eq_window.h"
#include "preset_manager.h"
#include "control_client.h"

static AppIndicator* indicator = NULL;
static EqWindow* eq_win = NULL;
static PresetManager* pm = NULL;
static int control_fd = -1;

static void on_quit(GSimpleAction* action, GVariant* param, gpointer data) {
    (void)action; (void)param; (void)data;
    if (eq_win) eq_window_destroy(eq_win);
    gtk_main_quit();
}

static void on_show(GSimpleAction* action, GVariant* param, gpointer data) {
    (void)action; (void)param; (void)data;
    if (eq_win) eq_window_present(eq_win);
}

static GActionEntry app_entries[] = {
    { "show", on_show, NULL, NULL, NULL },
    { "quit", on_quit, NULL, NULL, NULL },
};

static void on_band_changed(void* userdata, int band, float gain_db) {
    (void)userdata;
    if (control_fd >= 0)
        control_send_band_gain(control_fd, band, gain_db);
}

static void on_preamp_changed(void* userdata, float preamp_db) {
    (void)userdata;
    if (control_fd >= 0)
        control_send_preamp(control_fd, preamp_db);
}

static void on_bypass_changed(void* userdata, bool bypass) {
    (void)userdata;
    if (control_fd >= 0)
        control_send_bypass(control_fd, bypass);
}

static void on_preset_applied(void* userdata, const radioform_preset_t* preset) {
    (void)userdata;
    if (control_fd >= 0)
        control_send_preset(control_fd, preset);
}

static void on_indicator_activate(GtkWidget* widget, gpointer data) {
    (void)widget; (void)data;
    if (eq_win) eq_window_present(eq_win);
}

static void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        gtk_main_quit();
    }
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    gtk_init(&argc, &argv);

    pm = preset_manager_new();
    eq_win = eq_window_new();

    eq_window_set_callbacks(eq_win,
        on_band_changed, NULL,
        on_preamp_changed, NULL,
        on_bypass_changed, NULL,
        on_preset_applied, NULL);

    eq_window_load_preset(eq_win, preset_manager_current(pm));

    indicator = app_indicator_new(
        "radioform",
        "radioform",
        APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_menu(indicator, NULL);

    GtkWidget* menu = gtk_menu_new();

    GtkWidget* show_item = gtk_menu_item_new_with_label("Show EQ");
    g_signal_connect(show_item, "activate", G_CALLBACK(on_indicator_activate), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), show_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

    GtkWidget* quit_item = gtk_menu_item_new_with_label("Quit Radioform");
    g_signal_connect(quit_item, "activate", G_CALLBACK(on_quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);

    gtk_widget_show_all(menu);
    app_indicator_set_menu(indicator, GTK_MENU(menu));

    control_fd = control_connect();
    if (control_fd < 0)
        fprintf(stderr, "[radioform-ui] Warning: Could not connect to radioform-filter\n");

    gtk_main();

    if (control_fd >= 0) close(control_fd);
    preset_manager_free(pm);
    return 0;
}
