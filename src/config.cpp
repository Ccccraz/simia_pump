#include "config.h"
#include "Preferences.h"

namespace simia
{
Preferences prefs{};

void init_device_id_nickname_config()
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
    config.wifi.wifi_requirement = simia::default_wifi_requirement;
    simia::save_config(config);
}

void init_config()
{
    prefs.begin(pref_name, false);

    config_t config{
        .nickname = default_nickname,
        .device_id = default_device_id,
        .wifi =
            {
                .ssid = default_wifi_ssid,
                .password = default_wifi_pass,
                .wifi_requirement = default_wifi_requirement,
            },
        .start_mode = default_start_mode,
    };

    prefs.putBytes(pref_config_key, &config, sizeof(config));
    prefs.end();
}
config_t load_config()
{
    config_t config{};
    prefs.begin(pref_name, true);
    if (!prefs.isKey(pref_config_key))
    {
        prefs.end();
        init_config();
        prefs.begin(pref_name, true);
    }

    prefs.getBytes(pref_config_key, &config, sizeof(config));

    prefs.end();
    return config;
}
void save_config(config_t config)
{
    prefs.begin(pref_name, false);

    prefs.putBytes(pref_config_key, &config, sizeof(config));

    prefs.end();
}
} // namespace simia
