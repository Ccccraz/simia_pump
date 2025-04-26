#include "at8236_hid.h"
#include "preset.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <OneButton.h>
#include <USB.h>
#include <WiFi.h>

enum simia_start_mode : uint32_t
{
    NORMAL = 0,
    DOWNLOAD = 1,
    OTA = 2,
};

volatile uint32_t simia_start_mode{NORMAL};

// at8236 control pins
constexpr gpio_num_t first_pin{GPIO_NUM_8};
constexpr gpio_num_t second_pin{GPIO_NUM_18};

// system buttons
constexpr gpio_num_t start_pin{GPIO_NUM_35};
constexpr gpio_num_t stop_pin{GPIO_NUM_36};
constexpr gpio_num_t reverse_pin{GPIO_NUM_37};

// control functions
OneButton start_button{};
OneButton stop_button{};
OneButton reverse_button{};

AT8236HID pump(first_pin, second_pin, 1.0f);

volatile bool wifi_init = false;

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
    init_device_id();
    pump.device_id = load_device_id();
}

void init_pump_wifi_config()
{
    init_wifi_config();
}

void ota_update()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        save_simia_start_mode(simia_start_mode::NORMAL);
        NetworkClient client{};

        auto result = httpUpdate.update(client, "http://192.168.1.70:8080/firmware.bin");

        switch (result)
        {
        case HTTP_UPDATE_FAILED:
            break;
        case HTTP_UPDATE_NO_UPDATES:
            break;
        case HTTP_UPDATE_OK:
            break;
        }

        ESP.restart();
    }
}

static void simiapump_event_callback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == ARDUINO_USB_HID_SIMIA_PUMP_EVENTS)
    {
        auto *data = (arduino_usb_hid_simia_pump_event_data_t *)event_data;
        switch (event_id)
        {
        case ARDUINO_USB_HID_SIMIA_PUMP_SET_DEVICE_EVENT:
            save_device_id(pump.device_id);
            break;
        case ARDUINO_USB_HID_SIMIA_PUMP_SET_WIFI_EVENT:
            save_wifi_config(pump.ssid, pump.password, pump.need_wifi);
            break;
        case ARDUINO_USB_HID_SIMIA_PUMP_SET_OTA_EVENT:
            save_simia_start_mode(simia_start_mode::OTA);
            break;
        default:
            break;
        }
    }
}

void btn_task(void *param)
{
    // Configure buttons
    start_button.setup(start_pin);
    stop_button.setup(stop_pin);
    reverse_button.setup(reverse_pin);

    start_button.attachClick(start);
    stop_button.attachClick(stop);
    stop_button.setPressMs(5000);
    stop_button.attachLongPressStart(init_pump_wifi_config);
    reverse_button.attachClick(reverse);
    reverse_button.setPressMs(5000);
    reverse_button.attachLongPressStart(init_device_id);

    while (true) {
        start_button.tick();
        stop_button.tick();
        reverse_button.tick();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void setup()
{
    simia_start_mode = load_simia_start_mode();

    if (simia_start_mode == simia_start_mode::NORMAL)
    {
        // Configure pump
        pump.device_id = load_device_id();

        // handle pump events
        pump.onEvent(simiapump_event_callback);

        // Configure USB
        USB.manufacturerName("simia");
        USB.productName("pump_A100_v0.1.1");
        USB.begin();
        pump.begin();
    }
    else if (simia_start_mode == simia_start_mode::DOWNLOAD)
    {
        Serial.begin(115200);
    }
    else if (simia_start_mode == simia_start_mode::OTA)
    {
        auto wifi_config = load_wifi_config();

        if (wifi_config.need == 0x01)
        {
            WiFi.begin(wifi_config.ssid, wifi_config.password);
            while (WiFi.status() != WL_CONNECTED)
            {
                delay(1000);
            }
        }
    }
}

void loop()
{
    start_button.tick();
    stop_button.tick();
    reverse_button.tick();

    if (simia_start_mode == simia_start_mode::OTA)
    {
        ota_update();
    }

    vTaskDelay(pdMS_TO_TICKS(10));
}