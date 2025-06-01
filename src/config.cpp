#include "config.h"
#include "Preferences.h"

namespace simia
{
    Preferences prefs{};
    std::mutex prefs_mutex{};

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
        simia::save_config(config);
    }

    void init_config()
    {
        config_t config{
            .nickname = default_nickname,
            .device_id = default_device_id,
            .wifi =
                {
                    .ssid = default_wifi_ssid,
                    .password = default_wifi_pass,
                },
            .start_mode = default_start_mode,
        };

        save_config(config);
    }
    config_t load_config()
    {
        config_t config{};
        prefs.begin(pref_name, true);
        if (!prefs.isKey(nickname_key))
        {
            prefs.end();

            init_config();

            prefs.begin(pref_name, true);
        }

        config.nickname = prefs.getString(nickname_key, default_nickname);
        config.device_id = prefs.getUChar(device_id_key, default_device_id);
        config.wifi.ssid = prefs.getString(wifi_ssid_key, default_wifi_ssid);
        config.wifi.password = prefs.getString(wifi_pass_key, default_wifi_pass);
        config.start_mode = static_cast<start_mode_t>(prefs.getUChar(start_mode_key, static_cast<uint8_t>(default_start_mode)));

        prefs.end();
        return config;
    }
    void save_config(config_t config)
    {
        std::lock_guard<std::mutex> lock(prefs_mutex);
        prefs.begin(pref_name, false);

        prefs.putString(nickname_key, config.nickname);
        prefs.putUChar(device_id_key, config.device_id);
        prefs.putString(wifi_ssid_key, config.wifi.ssid);
        prefs.putString(wifi_pass_key, config.wifi.password);
        prefs.putUChar(start_mode_key, static_cast<uint8_t>(config.start_mode));

        prefs.end();
    }
} // namespace simia
