#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// at8236 control pins
constexpr const gpio_num_t first_pin{GPIO_NUM_8};
constexpr const gpio_num_t second_pin{GPIO_NUM_18};

// system buttons
constexpr const gpio_num_t start_pin{GPIO_NUM_35};
constexpr const gpio_num_t stop_pin{GPIO_NUM_36};
constexpr const gpio_num_t reverse_pin{GPIO_NUM_37};

namespace simia
{
enum class wifi_requirement_t : uint8_t
{
    REQUIRED = 0x00,
    NOT_REQUIRED = 0x01
};

// WiFi Config
struct wifi_config_t
{
    String ssid;
    String password;
    wifi_requirement_t wifi_requirement;
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
    start_mode_t start_mode;
};

constexpr const char *pref_name = "simia_pump_A100";
constexpr const char *pref_config_key = "config";

// Nickname
constexpr const char *default_nickname = "simia_pump";

// Device ID
constexpr const uint8_t default_device_id{0};

// WiFi
constexpr const char *default_wifi_ssid = "";
constexpr const char *default_wifi_pass = "";
constexpr const wifi_requirement_t default_wifi_requirement{wifi_requirement_t::NOT_REQUIRED};

// Start mode
constexpr const start_mode_t default_start_mode{start_mode_t::NORMAL};

// OTA
constexpr const char *default_ota_url = "";

void init_device_id_nickname_config();
void init_wifi_config();

void init_config();
config_t load_config();
void save_config(config_t config);
} // namespace simia

#endif // CONFIG_H