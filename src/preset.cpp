#include "preset.h"
#include "Preferences.h"

Preferences prefs{};

const char *pref_name = "simia_pump_A100";

const char *pref_device_key = "device_id";
const char *pref_wifi_ssid = "wifi_ssid";
const char *pref_wifi_pass = "wifi_pass";
const char *pref_need_wifi = "need_wifi";

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
    prefs.putString(pref_wifi_ssid, "");
    prefs.putString(pref_wifi_pass, "");
    prefs.putBool(pref_need_wifi, false);
    prefs.end();
}

simia_wifi_config_t load_wifi_config()
{
    simia_wifi_config_t config{};
    prefs.begin(pref_name, true);

    if (!prefs.isKey(pref_wifi_ssid))
    {
        prefs.end();
        init_wifi_config();
        prefs.begin(pref_name, true);
    }

    config.ssid = prefs.getString(pref_wifi_ssid, "");
    config.password = prefs.getString(pref_wifi_pass, "");
    config.need = prefs.getBool(pref_need_wifi, false);

    return config;
}

void save_wifi_config(simia_wifi_config_t config)
{
    prefs.begin(pref_name, false);
    prefs.putString(pref_wifi_ssid, config.ssid);
    prefs.putString(pref_wifi_pass, config.password);
    prefs.putBool(pref_need_wifi, config.need);
    prefs.end();
}