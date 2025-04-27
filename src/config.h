#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// at8236 control pins
constexpr const uint8_t first_pin{8};
constexpr const uint8_t second_pin{18};

// system buttons
constexpr const gpio_num_t start_pin{GPIO_NUM_35};
constexpr const gpio_num_t stop_pin{GPIO_NUM_36};
constexpr const gpio_num_t reverse_pin{GPIO_NUM_37};

namespace simia
{
// WiFi Config
struct wifi_config_t
{
    String ssid;
    String password;
};

enum class wifi_requirement_t : uint8_t
{
    REQUIRED = 0x00,
    NOT_REQUIRED = 0x01
};

// Start Mode
enum class start_mode_t : uint8_t
{
    NORMAL = 0x00,
    FLASH = 0x01,
    ACTIVE_OTA = 0x02,
};

// Config
struct config_t
{
    String nickname;
    uint8_t device_id;
    wifi_config_t wifi;
    wifi_requirement_t wifi_requirement;
    start_mode_t start_mode;
    String ota_url;
};

constexpr const char *pref_name = "simia_pump_A100";

// Nickname
constexpr const char *pref_nickname_key = "nickname";
constexpr const char *default_nickname = "simia pump";

// Device ID
constexpr const char *pref_device_id_key = "device_id";
constexpr const uint8_t default_device_id{0};

// WiFi
constexpr const char *pref_wifi_ssid_key = "wifi_ssid";
constexpr const char *pref_wifi_pass_key = "wifi_pass";
constexpr const char *pref_wifi_requirement_key = "wifi_requirement";

constexpr const char *default_wifi_ssid = "B&C";
constexpr const char *default_wifi_pass = "325325325";
constexpr const wifi_requirement_t default_wifi_requirement{wifi_requirement_t::REQUIRED};

// Start mode
constexpr const char *pref_start_mode = "start_mode";
constexpr const start_mode_t default_start_mode{start_mode_t::NORMAL};

// OTA
constexpr const char *pref_ota_url_key = "ota_url";
constexpr const char *default_ota_url = "";

void init_device_id();
void init_wifi_config();

void init_config();
config_t load_config();
void save_config(config_t config);
} // namespace simia

#endif // CONFIG_H