#include "config.h"
#include "Preferences.h"

namespace simia
{
Preferences prefs{};


void init_config()
{
    prefs.begin(pref_name, false);
    prefs.putUChar(pref_device_id_key, default_device_id);
    prefs.end();

    prefs.begin(pref_name, false);
    prefs.putUChar(pref_wifi_requirement_key,
                   static_cast<std::underlying_type_t<wifi_requirement_t>>(default_wifi_requirement));
    prefs.end();

    prefs.begin(pref_name, false);
    prefs.putUChar(pref_wifi_requirement_key,
                   static_cast<std::underlying_type_t<wifi_requirement_t>>(default_wifi_requirement));
    prefs.end();

    prefs.begin(pref_name, false);
    prefs.putUChar(pref_start_mode, static_cast<std::underlying_type_t<start_mode_t>>(default_start_mode));
    prefs.end();
}
config_t load_config()
{
    config_t config{};
    prefs.begin(pref_name, true);
    if (!prefs.isKey(pref_device_id_key))
    {
        prefs.end();
        init_config();
        prefs.begin(pref_name, true);
    }
    config.device_id = prefs.getUChar(pref_device_id_key, default_device_id);
    config.wifi.ssid = prefs.getString(pref_wifi_ssid_key, default_wifi_ssid);
    config.wifi.password = prefs.getString(pref_wifi_pass_key, default_wifi_pass);
    config.wifi_requirement = static_cast<wifi_requirement_t>(prefs.getUChar(
        pref_wifi_requirement_key, static_cast<std::underlying_type_t<wifi_requirement_t>>(default_wifi_requirement)));
    config.start_mode = static_cast<start_mode_t>(
        prefs.getUChar(pref_start_mode, static_cast<std::underlying_type_t<start_mode_t>>(default_start_mode)));

    prefs.end();
    return config;
}
void save_config(config_t config)
{
    prefs.begin(pref_name, false);
    prefs.putUChar(pref_device_id_key, config.device_id);

    prefs.putString(pref_wifi_ssid_key, config.wifi.ssid);
    prefs.putString(pref_wifi_pass_key, config.wifi.password);
    prefs.putUChar(pref_wifi_requirement_key,
                   static_cast<std::underlying_type_t<wifi_requirement_t>>(config.wifi_requirement));
    prefs.putUChar(pref_start_mode, static_cast<std::underlying_type_t<start_mode_t>>(config.start_mode));

    prefs.end();
}
} // namespace simia
