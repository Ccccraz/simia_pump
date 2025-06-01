#include "at8236_hid.h"
#include "config.h"
#include "ota.h"

#include <Arduino.h>
#include <OneButton.h>

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_https_ota.h>

#include <USB.h>

#include <esp32-hal-tinyusb.h>

simia::config_t config{};

AT8236HID pump(first_pin, second_pin, 1.0f, config);

// Callback for button events
static void start()
{
    pump.add_task(0);
}

void stop()
{
    pump.stop(true);
}

void reverse()
{
    pump.reverse();
}

void init_device_id()
{
    simia::init_device_id_nickname_config();
    pump.set_device_info(simia::default_device_id, simia::default_nickname);
}

// Callback for simiapump events
void set_device_id_nickname_cb(void *param)
{
    auto device_info = *(device_info_t *)param;
    config.device_id = device_info.device_id;
    config.nickname = String(device_info.nickname, device_info.nickname_len);
    simia::save_config(config);
}

void set_wifi_cb(void *param)
{
    auto wifi_info = *(wifi_info_t *)param;
    auto wifi_ssid = String(wifi_info.ssid, wifi_info.ssid_len);
    auto wifi_password = String(wifi_info.password, wifi_info.password_len);

    config.wifi.ssid = wifi_ssid;
    config.wifi.password = wifi_password;
    simia::save_config(config);
}

void enable_flash_mode()
{
    usb_persist_restart(RESTART_BOOTLOADER);
}

void enable_active_ota()
{
    config.start_mode = simia::start_mode_t::ACTIVE_OTA;
    simia::save_config(config);
}

void set_start_mode_cb(void *param)
{
    auto start_mode = *(simia::start_mode_t *)param;
    switch (start_mode)
    {
    case simia::start_mode_t::FLASH:
        usb_persist_restart(RESTART_BOOTLOADER);
        break;

    case simia::start_mode_t::ACTIVE_OTA:
        enable_active_ota();
        break;

    default:
        break;
    }
}

// simiapump events handler
static void simiapump_event_callback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == ARDUINO_USB_HID_SIMIA_PUMP_EVENTS)
    {
        switch (event_id)
        {
        case ARDUINO_USB_HID_SIMIA_PUMP_SET_DEVICE_EVENT:
            set_device_id_nickname_cb(event_data);
            break;
        case ARDUINO_USB_HID_SIMIA_PUMP_SET_WIFI_EVENT:
            set_wifi_cb(event_data);
            break;
        case ARDUINO_USB_HID_SIMIA_PUMP_SET_START_MODE_EVENT:
            set_start_mode_cb(event_data);
            break;
        default:
            break;
        }
    }
}

void btn_task(void *param)
{
    // control functions
    OneButton start_button{};
    OneButton stop_button{};
    OneButton reverse_button{};

    start_button.setup(start_pin);
    stop_button.setup(stop_pin);
    reverse_button.setup(reverse_pin);

    start_button.attachClick(start);
    stop_button.attachClick(stop);
    stop_button.setPressMs(5000);
    stop_button.attachLongPressStart(simia::init_wifi_config);

    reverse_button.attachClick(reverse);
    reverse_button.setPressMs(5000);
    reverse_button.attachLongPressStart(init_device_id);
    while (true)
    {
        start_button.tick();
        stop_button.tick();
        reverse_button.tick();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void normal_start(simia::config_t config)
{
    pump.set_device_info(config.device_id, config.nickname);
    pump.onEvent(simiapump_event_callback);

    USB.manufacturerName("simia");
    USB.productName("pump_A100_v0.1.1");

    USB.begin();
    pump.begin();
}

void active_ota_start(simia::config_t config)
{
    config.start_mode = simia::start_mode_t::NORMAL;
    simia::save_config(config);

    if (config.wifi.ssid.isEmpty())
        return;

    WiFi.mode(WIFI_STA);
    WiFi.begin(config.wifi.ssid, config.wifi.password);

    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ota_update();
}

void setup()
{
    config = simia::load_config();
    pump.set_config(config);

    switch (config.start_mode)
    {
    case simia::start_mode_t::NORMAL:
        normal_start(config);
        break;
    case simia::start_mode_t::ACTIVE_OTA:
        active_ota_start(config);
        break;
    default:
        break;
    }

    xTaskCreatePinnedToCore(btn_task, "btn_task", 1024 * 8, nullptr, 1, nullptr, 1);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(portMAX_DELAY));
}