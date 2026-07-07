#ifndef RADIOFORM_PRESET_MANAGER_H
#define RADIOFORM_PRESET_MANAGER_H

#include "radioform_types.h"

typedef struct PresetManager PresetManager;

PresetManager* preset_manager_new(void);
void preset_manager_free(PresetManager* pm);
const radioform_preset_t* preset_manager_current(PresetManager* pm);
int preset_manager_get_all(PresetManager* pm, radioform_preset_t** out, int* count);
int preset_manager_load(PresetManager* pm, const char* path);
int preset_manager_save(PresetManager* pm, const char* path, const radioform_preset_t* preset);
int preset_manager_apply(PresetManager* pm, const char* name);

#endif
