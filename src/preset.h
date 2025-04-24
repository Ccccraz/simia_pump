#ifndef PRESET_H
#define PRESET_H

#include <Arduino.h>

void init_prefs();

uint32_t load_device_id();
void save_device_id(uint32_t id);

#endif