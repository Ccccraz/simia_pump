#include "at8236_hid.h"
#include "preset.h"
#include <Arduino.h>
#include <OneButton.h>
#include <USB.h>
#include <WiFi.h>

const char *ssid = "ccccr";
const char *password = "12345678";

// at8236 control pins
constexpr uint8_t first_pin{8};
constexpr uint8_t second_pin{18};

// system buttons
constexpr uint8_t start_pin{35};
constexpr uint8_t stop_pin{36};
constexpr uint8_t reverse_pin{37};

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

static void simiapump_event_callback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == ARDUINO_USB_HID_SIMIA_PUMP_EVENTS)
    {
        auto *data = (arduino_usb_hid_simia_pump_event_data_t *)event_data;
        switch (event_id)
        {
        case ARDUINO_USB_HID_SIMIA_PUMP_SET_FEATURE_EVENT:
            save_device_id(pump.device_id);
            break;
        default:
            break;
        }
    }
}

void setup()
{
    auto wifi_config = load_wifi_config();
    if (wifi_config.need)
    {
        WiFi.begin(wifi_config.ssid, wifi_config.password);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(1000);
        }
    }
    // Configure pump
    pump.device_id = load_device_id();

    // handle pump events
    pump.onEvent(simiapump_event_callback);

    // Configure USB
    USB.manufacturerName("simia");
    USB.productName("pump_A100_v0.1.1");
    USB.begin();
    pump.begin();

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
}

void loop()
{
    start_button.tick();
    stop_button.tick();
    reverse_button.tick();

    vTaskDelay(pdMS_TO_TICKS(10));
}