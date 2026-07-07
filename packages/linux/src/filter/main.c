#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <pipewire/pipewire.h>
#include <pipewire/filter.h>
#include <spa/param/audio/format-utils.h>
#include "radioform_dsp.h"

struct data {
    struct pw_main_loop* loop;
    struct pw_core* core;
    struct pw_filter* filter;
    struct spa_hook filter_listener;
    radioform_dsp_engine_t* dsp;
    uint32_t sample_rate;
    uint32_t channels;
    int control_fd;
};

static void on_process(void* userdata) {
    struct data* d = userdata;

    struct pw_buffer* in_buf = pw_filter_dequeue_buffer(d->filter, PW_DIRECTION_INPUT);
    struct pw_buffer* out_buf = pw_filter_dequeue_buffer(d->filter, PW_DIRECTION_OUTPUT);
    if (!in_buf || !out_buf) return;

    struct spa_buffer* in = in_buf->buffer;
    struct spa_buffer* out = out_buf->buffer;

    if (in->n_datas < 1 || !in->datas[0].data ||
        out->n_datas < 1 || !out->datas[0].data) {
        goto done;
    }

    uint32_t n_frames = in->datas[0].chunk->size / (d->channels * sizeof(float));
    if (n_frames == 0) goto done;

    float* in_samples = (float*)in->datas[0].data;
    float* out_samples = (float*)out->datas[0].data;

    radioform_dsp_process_interleaved(d->dsp, in_samples, out_samples, n_frames);

done:
    if (in_buf) pw_filter_queue_buffer(d->filter, in_buf);
    if (out_buf) pw_filter_queue_buffer(d->filter, out_buf);
}

static const struct pw_filter_events filter_events = {
    PW_VERSION_FILTER_EVENTS,
    .process = on_process,
};

static int create_filter(struct pw_core* core, struct data* d) {
    struct pw_properties* props = pw_properties_new(
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CATEGORY, "Filter",
        PW_KEY_MEDIA_ROLE, "DSP",
        PW_KEY_NODE_NAME, "radioform-eq",
        PW_KEY_NODE_DESCRIPTION, "Radioform 10-Band Equalizer",
        PW_KEY_NODE_VIRTUAL, "true",
        NULL
    );

    struct spa_audio_info_raw info = {
        .format = SPA_AUDIO_FORMAT_F32,
        .rate = d->sample_rate,
        .channels = d->channels,
    };

    uint8_t params[1024];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(params, sizeof(params));
    const struct spa_pod* pod = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &info);

    d->filter = pw_filter_new(core, "radioform-eq-filter", props);
    if (!d->filter) return -1;

    pw_filter_add_listener(d->filter, &d->filter_listener, &filter_events, d);

    if (pw_filter_connect(d->filter,
            PW_FILTER_FLAG_MAP_BUFFERS,
            &pod, 1) < 0) {
        pw_filter_destroy(d->filter);
        d->filter = NULL;
        return -1;
    }

    return 0;
}

static void* control_thread(void* userdata) {
    struct data* d = userdata;
    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, "/tmp/radioform-control.sock", sizeof(addr.sun_path) - 1);

    d->control_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (d->control_fd < 0) return NULL;

    unlink("/tmp/radioform-control.sock");
    if (bind(d->control_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0
        || listen(d->control_fd, 1) < 0) {
        close(d->control_fd);
        d->control_fd = -1;
        return NULL;
    }
    chmod("/tmp/radioform-control.sock", 0666);

    while (1) {
        struct sockaddr_un client;
        socklen_t len = sizeof(client);
        int fd = accept(d->control_fd, (struct sockaddr*)&client, &len);
        if (fd < 0) break;

        char buf[4096];
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = 0;
            char* line = buf;
            while (line && *line) {
                char* nl = strchr(line, '\n');
                if (nl) *nl = 0;

                if (strncmp(line, "band ", 5) == 0) {
                    int idx; float val;
                    if (sscanf(line + 5, "%d %f", &idx, &val) == 2)
                        radioform_dsp_update_band_gain(d->dsp, idx, val);
                } else if (strncmp(line, "preamp ", 7) == 0) {
                    float val;
                    if (sscanf(line + 7, "%f", &val) == 1)
                        radioform_dsp_update_preamp(d->dsp, val);
                } else if (strncmp(line, "bypass ", 7) == 0) {
                    int val;
                    if (sscanf(line + 7, "%d", &val) == 1)
                        radioform_dsp_set_bypass(d->dsp, val != 0);
                }

                line = nl ? nl + 1 : NULL;
            }
        }
        close(fd);
    }
    return NULL;
}

static volatile int running = 1;

static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

int main(int argc, char* argv[]) {
    struct data d = {
        .sample_rate = 48000,
        .channels = 2,
        .control_fd = -1,
    };

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--rate") == 0 && i + 1 < argc)
            d.sample_rate = atoi(argv[i + 1]);
        if (strcmp(argv[i], "--channels") == 0 && i + 1 < argc)
            d.channels = atoi(argv[i + 1]);
    }

    pw_init(&argc, &argv);

    d.loop = pw_main_loop_new(NULL);
    struct pw_context* context = pw_context_new(pw_main_loop_get_loop(d.loop), NULL, 0);
    d.core = pw_context_connect(context, NULL, 0);
    if (!d.core) { fprintf(stderr, "Failed to connect to PipeWire\n"); return 1; }

    d.dsp = radioform_dsp_create(d.sample_rate);
    if (!d.dsp) { fprintf(stderr, "Failed to create DSP engine\n"); return 1; }
    radioform_dsp_enable_denormal_suppression();

    radioform_preset_t flat;
    radioform_dsp_preset_init_flat(&flat);
    radioform_dsp_apply_preset(d.dsp, &flat);

    if (create_filter(d.core, &d) < 0) {
        fprintf(stderr, "Failed to create PipeWire filter\n");
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    pthread_t ctl;
    pthread_create(&ctl, NULL, control_thread, &d);

    printf("radioform-filter: running (rate=%u Hz, channels=%u)\n", d.sample_rate, d.channels);
    printf("radioform-filter: control socket at /tmp/radioform-control.sock\n");
    printf("radioform-filter: Node 'Radioform 10-Band Equalizer' created\n");
    printf("radioform-filter: Use pw-link to route audio through the filter\n");
    printf("radioform-filter:   pw-link <source> radioform-eq:input_0\n");
    printf("radioform-filter:   pw-link radioform-eq:output_0 <sink>\n");

    while (running) {
        pw_main_loop_run(d.loop);
    }

    radioform_dsp_destroy(d.dsp);
    if (d.filter) pw_filter_destroy(d.filter);
    pw_core_disconnect(d.core);
    pw_context_destroy(context);
    pw_main_loop_destroy(d.loop);
    pw_deinit();

    if (d.control_fd >= 0) {
        close(d.control_fd);
        unlink("/tmp/radioform-control.sock");
    }

    return 0;
}
