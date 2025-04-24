#ifndef PRESET_H
#define PRESET_H

#include <Arduino.h>

struct simia_wifi_config_t
{
    bool need;
    String ssid;
    String password;
};

void init_pump_device_id();

uint32_t load_device_id();
void save_device_id(uint32_t id);

void init_wifi_config();

simia_wifi_config_t load_wifi_config();
void save_wifi_config(simia_wifi_config_t config);

#endif