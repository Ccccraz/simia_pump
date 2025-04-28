#include "config.h"
#include "Preferences.h"

namespace simia
{
Preferences prefs{};

void init_device_id()
{
    auto config = simia::load_config();
    config.device_id = default_device_id;
    simia::save_config(config);
}

void init_wifi_config()
{
    auto config = simia::load_config();
    config.wifi.ssid = simia::default_wifi_ssid;
    config.wifi.password = simia::default_wifi_pass;
    simia::save_config(config);
}

void init_config()
{
    prefs.begin(pref_name, false);

    // Nickname
    prefs.putString(pref_nickname_key, default_nickname);

    // Device ID
    prefs.putUChar(pref_device_id_key, default_device_id);

    // WiFi
    prefs.putString(pref_wifi_ssid_key, default_wifi_ssid);
    prefs.putString(pref_wifi_pass_key, default_wifi_pass);

    // WiFi Requirement
    prefs.putUChar(pref_wifi_requirement_key,
                   static_cast<std::underlying_type_t<wifi_requirement_t>>(default_wifi_requirement));

    // Start Mode
    prefs.putUChar(pref_start_mode, static_cast<std::underlying_type_t<start_mode_t>>(default_start_mode));

    // OTA URL
    prefs.putString(pref_ota_url_key, default_ota_url);
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
    // Nickname
    config.nickname = prefs.getString(pref_nickname_key, default_nickname);

    // Device ID
    config.device_id = prefs.getUChar(pref_device_id_key, default_device_id);

    // WiFi Information
    config.wifi.ssid = prefs.getString(pref_wifi_ssid_key, default_wifi_ssid);
    config.wifi.password = prefs.getString(pref_wifi_pass_key, default_wifi_pass);

    // WiFi Requirement
    config.wifi_requirement = static_cast<wifi_requirement_t>(prefs.getUChar(
        pref_wifi_requirement_key, static_cast<std::underlying_type_t<wifi_requirement_t>>(default_wifi_requirement)));

    // Start Mode
    config.start_mode = static_cast<start_mode_t>(
        prefs.getUChar(pref_start_mode, static_cast<std::underlying_type_t<start_mode_t>>(default_start_mode)));

    // OTA URL
    config.ota_url = prefs.getString(pref_ota_url_key, default_ota_url);

    prefs.end();
    return config;
}
void save_config(config_t config)
{
    prefs.begin(pref_name, false);
    // Nickname
    prefs.putString(pref_nickname_key, config.nickname);

    // Device ID
    prefs.putUChar(pref_device_id_key, config.device_id);

    // WiFi Information
    prefs.putString(pref_wifi_ssid_key, config.wifi.ssid);
    prefs.putString(pref_wifi_pass_key, config.wifi.password);

    // WiFi Requirement
    prefs.putUChar(pref_wifi_requirement_key,
                   static_cast<std::underlying_type_t<wifi_requirement_t>>(config.wifi_requirement));
    // Start Mode
    prefs.putUChar(pref_start_mode, static_cast<std::underlying_type_t<start_mode_t>>(config.start_mode));

    // OTA URL
    prefs.putString(pref_ota_url_key, config.ota_url);

    prefs.end();
}
} // namespace simia
