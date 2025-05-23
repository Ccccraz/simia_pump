#include "at8236_hid.h"
#include "config.h"
#include "mqtt.h"
#include "ota.h"

#include <Arduino.h>
#include <OneButton.h>

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_https_ota.h>

#include <USB.h>

WiFiClientSecure esp_client;
PubSubClient mqtt_client(esp_client);

AT8236HID pump(first_pin, second_pin, 1.0f);

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
    simia::init_device_id();
    pump.set_device_id(simia::default_device_id);
}

// Callback for simiapump events
void set_device_id_cb(void *param)
{
    auto device_id = *(uint8_t *)param;
    auto config = simia::load_config();
    config.device_id = device_id;
    simia::save_config(config);
}

void set_wifi_cb(void *param)
{
    auto wifi_info = *(wifi_info_t *)param;
    auto wifi_ssid = String(wifi_info.ssid, wifi_info.ssid_len);
    auto wifi_password = String(wifi_info.password, wifi_info.password_len);

    auto config = simia::load_config();
    config.wifi.ssid = wifi_ssid;
    config.wifi.password = wifi_password;
    simia::save_config(config);
}

void set_ota_cb(void *param)
{
    auto ota_info = *(ota_info_t *)param;
    String ota_url = String(ota_info.url, ota_info.url_len);
    auto config = simia::load_config();
    config.ota_url = ota_url;
    simia::save_config(config);
}

void enable_wifi_cb()
{
    auto config = simia::load_config();
    config.wifi_requirement = simia::wifi_requirement_t::REQUIRED;
    simia::save_config(config);
}

void disable_wifi_cb()
{
    auto config = simia::load_config();
    config.wifi_requirement = simia::wifi_requirement_t::NOT_REQUIRED;
    simia::save_config(config);
}

void enable_flash_cb()
{
    auto config = simia::load_config();
    config.start_mode = simia::start_mode_t::FLASH;
    simia::save_config(config);
}

// simiapump events handler
static void simiapump_event_callback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == ARDUINO_USB_HID_SIMIA_PUMP_EVENTS)
    {
        switch (event_id)
        {
        case ARDUINO_USB_HID_SIMIA_PUMP_SET_DEVICE_EVENT:
            set_device_id_cb(event_data);
            break;
        case ARDUINO_USB_HID_SIMIA_PUMP_SET_WIFI_EVENT:
            set_wifi_cb(event_data);
            break;
        case ARDUINO_USB_HID_SIMIA_PUMP_SET_OTA_EVENT:
            set_ota_cb(event_data);
            break;
        case ARDUINO_USB_HID_SIMIA_PUMP_ENABLE_WIFI_EVENT:
            enable_wifi_cb();
            break;
        case ARDUINO_USB_HID_SIMIA_PUMP_DISABLE_WIFI_EVENT:
            disable_wifi_cb();
            break;
        case ARDUINO_USB_HID_SIMIA_PUMP_ENABLE_FLASH_EVENT:
            enable_flash_cb();
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

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
    ota_update(mqtt_client);
}

void connect_mqtt()
{
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setKeepAlive(60);
    mqtt_client.setCallback(mqtt_callback);

    while (!mqtt_client.connected())
    {
        String client_id = "esp32-client-" + String(WiFi.macAddress());
        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password))
        {
            mqtt_client.subscribe(mqtt_topic_sub);
            mqtt_client.publish(mqtt_topic_pub, "hello world!");
            vTaskDelay(5000);
        }
    }
}

void mqtt_task(void *param)
{
    connect_mqtt();

    while (true)
    {
        if (!mqtt_client.connected())
        {
            connect_mqtt();
        }

        mqtt_client.loop();

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void normal_start(simia::config_t config)
{
    pump.set_device_id(config.device_id);
    pump.onEvent(simiapump_event_callback);

    USB.manufacturerName("simia");
    USB.productName("pump_A100_v0.1.1");

    USB.begin();
    pump.begin();

    if (config.wifi_requirement == simia::wifi_requirement_t::REQUIRED)
    {
        if (!config.wifi.ssid.isEmpty())
        {
            WiFi.mode(WIFI_STA);
            WiFi.begin(config.wifi.ssid, config.wifi.password);

            for (int i = 0; i < 10; i++)
            {
                if (WiFi.status() == WL_CONNECTED)
                {
                    esp_client.setCACert(ca_cert);
                    xTaskCreatePinnedToCore(mqtt_task, "mqtt_task", 1024 * 8, nullptr, 1, nullptr, 1);
                    break;
                }
                else
                {
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }
            }

            if (WiFi.status() != WL_CONNECTED)
            {
                WiFi.disconnect();
                WiFi.mode(WIFI_OFF);
            }
        }
    }
}

void flash_start(simia::config_t config)
{
    config.start_mode = simia::start_mode_t::NORMAL;
    simia::save_config(config);
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

    ota_without_mqtt();
}

void setup()
{
    auto config = simia::load_config();

    switch (config.start_mode)
    {
    case simia::start_mode_t::NORMAL:
        normal_start(config);
        break;
    case simia::start_mode_t::FLASH:
        flash_start(config);
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