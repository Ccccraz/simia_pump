#include "preset.h"
#include "Preferences.h"

Preferences prefs{};

const char *pref_name = "simia_pump_A100";

const char *pref_device_key = "device_id";
const char *pref_wifi_ssid = "wifi_ssid";
const char *pref_wifi_pass = "wifi_pass";
const char *pref_need_wifi = "need_wifi";

const char *pref_simia_start_mode = "simia_mode";

void init_pump_device_id()
{
    prefs.begin(pref_name, false);
    prefs.putUInt(pref_device_key, 0);
    prefs.end();
}

uint32_t load_device_id()
{
    prefs.begin(pref_name, true);

    if (!prefs.isKey(pref_device_key))
    {
        prefs.end();
        init_pump_device_id();
        prefs.begin(pref_name, true);
    }

    uint32_t id = prefs.getUInt(pref_device_key, 0);
    prefs.end();
    return id;
}

void save_device_id(uint32_t id)
{
    prefs.begin(pref_name, false);
    prefs.putUInt(pref_device_key, id);
    prefs.end();
}

void init_wifi_config()
{
    prefs.begin(pref_name, false);
    prefs.putString(pref_wifi_ssid, "B&C");
    prefs.putString(pref_wifi_pass, "325325325");
    prefs.putUChar(pref_need_wifi, 0x01);
    prefs.end();
}

simia_wifi_config_t load_wifi_config()
{
    simia_wifi_config_t config{};
    prefs.begin(pref_name, true);

    // if (!prefs.isKey(pref_wifi_ssid))
    // {
    prefs.end();
    init_wifi_config();
    prefs.begin(pref_name, true);
    // }

    config.ssid = prefs.getString(pref_wifi_ssid, "");
    config.password = prefs.getString(pref_wifi_pass, "");
    config.need = prefs.getUChar(pref_need_wifi, 0x01);

    return config;
}

void save_wifi_config(String ssid, String password, uint8_t need)
{
    prefs.begin(pref_name, false);
    prefs.putString(pref_wifi_ssid, ssid);
    prefs.putString(pref_wifi_pass, password);
    prefs.putUChar(pref_need_wifi, need);
    prefs.end();
}

uint32_t load_simia_start_mode()
{
    prefs.begin(pref_name, true);
    uint32_t mode = prefs.getUInt(pref_simia_start_mode, 0);
    prefs.end();
    return mode;
}

void save_simia_start_mode(uint32_t mode)
{
    prefs.begin(pref_name, false);
    prefs.putUInt(pref_simia_start_mode, mode);
    prefs.end();
}