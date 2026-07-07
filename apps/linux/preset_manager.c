#include "preset_manager.h"
#include "radioform_dsp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <json-c/json.h>

#define PRESETS_DIR "/usr/share/radioform/presets"
#define USER_PRESETS_DIR getenv("HOME") ? "" : ""

struct PresetManager {
    radioform_preset_t current;
    radioform_preset_t* presets;
    int num_presets;
};

static int parse_preset_file(const char* path, radioform_preset_t* preset) {
    FILE* f = fopen(path, "r");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len <= 0) { fclose(f); return -1; }

    char* buf = malloc(len + 1);
    if (fread(buf, 1, len, f) != (size_t)len) { free(buf); fclose(f); return -1; }
    buf[len] = 0;
    fclose(f);

    struct json_object* root = json_tokener_parse(buf);
    free(buf);
    if (!root) return -1;

    radioform_dsp_preset_init_flat(preset);

    struct json_object* val;
    if (json_object_object_get_ex(root, "name", &val))
        strncpy(preset->name, json_object_get_string(val), sizeof(preset->name) - 1);
    if (json_object_object_get_ex(root, "preamp_db", &val))
        preset->preamp_db = json_object_get_double(val);
    if (json_object_object_get_ex(root, "limiter_enabled", &val))
        preset->limiter_enabled = json_object_get_boolean(val);
    if (json_object_object_get_ex(root, "limiter_threshold_db", &val))
        preset->limiter_threshold_db = json_object_get_double(val);

    struct json_object* bands;
    if (json_object_object_get_ex(root, "bands", &bands)) {
        int n = json_object_array_length(bands);
        if (n > 10) n = 10;
        preset->num_bands = n;
        for (int i = 0; i < n; i++) {
            struct json_object* band = json_object_array_get_idx(bands, i);
            if (json_object_object_get_ex(band, "frequency_hz", &val))
                preset->bands[i].frequency_hz = json_object_get_double(val);
            if (json_object_object_get_ex(band, "gain_db", &val))
                preset->bands[i].gain_db = json_object_get_double(val);
            if (json_object_object_get_ex(band, "q_factor", &val))
                preset->bands[i].q_factor = json_object_get_double(val);
            if (json_object_object_get_ex(band, "enabled", &val))
                preset->bands[i].enabled = json_object_get_boolean(val);

            struct json_object* type;
            if (json_object_object_get_ex(band, "filter_type", &type)) {
                const char* s = json_object_get_string(type);
                if (strcmp(s, "lowShelf") == 0) preset->bands[i].type = RADIOFORM_FILTER_LOW_SHELF;
                else if (strcmp(s, "highShelf") == 0) preset->bands[i].type = RADIOFORM_FILTER_HIGH_SHELF;
                else if (strcmp(s, "lowPass") == 0) preset->bands[i].type = RADIOFORM_FILTER_LOW_PASS;
                else if (strcmp(s, "highPass") == 0) preset->bands[i].type = RADIOFORM_FILTER_HIGH_PASS;
                else if (strcmp(s, "notch") == 0) preset->bands[i].type = RADIOFORM_FILTER_NOTCH;
                else if (strcmp(s, "bandPass") == 0) preset->bands[i].type = RADIOFORM_FILTER_BAND_PASS;
                else preset->bands[i].type = RADIOFORM_FILTER_PEAK;
            }
        }
    }

    json_object_put(root);
    return 0;
}

PresetManager* preset_manager_new(void) {
    PresetManager* pm = calloc(1, sizeof(PresetManager));
    radioform_dsp_preset_init_flat(&pm->current);
    strcpy(pm->current.name, "Flat");

    // Scan preset dirs
    const char* search_dirs[] = {
        PRESETS_DIR,
        NULL
    };

    // Add user presets dir
    const char* home = getenv("HOME");
    char user_dir[256];
    if (home) {
        snprintf(user_dir, sizeof(user_dir), "%s/.local/share/radioform/presets", home);
        search_dirs[1] = user_dir;
    }

    for (int d = 0; d < 2 && search_dirs[d]; d++) {
        DIR* dir = opendir(search_dirs[d]);
        if (!dir) continue;
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type != DT_REG && entry->d_type != DT_LNK) continue;
            const char* ext = strrchr(entry->d_name, '.');
            if (!ext || strcasecmp(ext, ".json") != 0) continue;

            char path[512];
            snprintf(path, sizeof(path), "%s/%s", search_dirs[d], entry->d_name);
            pm->presets = realloc(pm->presets, sizeof(radioform_preset_t) * (pm->num_presets + 1));
            if (parse_preset_file(path, &pm->presets[pm->num_presets]) == 0)
                pm->num_presets++;
        }
        closedir(dir);
    }

    return pm;
}

void preset_manager_free(PresetManager* pm) {
    if (pm) {
        free(pm->presets);
        free(pm);
    }
}

const radioform_preset_t* preset_manager_current(PresetManager* pm) {
    return pm ? &pm->current : NULL;
}

int preset_manager_get_all(PresetManager* pm, radioform_preset_t** out, int* count) {
    if (!pm || !out || !count) return -1;
    *out = pm->presets;
    *count = pm->num_presets;
    return 0;
}

int preset_manager_apply(PresetManager* pm, const char* name) {
    if (!pm || !name) return -1;
    for (int i = 0; i < pm->num_presets; i++) {
        if (strcmp(pm->presets[i].name, name) == 0) {
            memcpy(&pm->current, &pm->presets[i], sizeof(radioform_preset_t));
            return 0;
        }
    }
    return -1;
}
