#include "config.h"
#include <Arduino.h>
#include <OneButton.h>
#include <Preferences.h>
#include <USB.h>
#include "preset.h"
#include "at8236_hid.h"

// at8236 control pins
constexpr uint8_t first_pin{8};
constexpr uint8_t second_pin{18};

// system buttons
constexpr uint8_t start_pin{35};
constexpr uint8_t stop_pin{36};
constexpr uint8_t reverse_pin{37};

// control functions
OneButton start_btn(start_pin, true);
OneButton stop_btn(stop_pin, true);
OneButton reverse_btn(reverse_pin, true);

AT8236HID pump(first_pin, second_pin, 1.0f);

void start()
{
    pump.add_task(0);
}

void stop()
{
    pump.stop();
}

void reverse()
{
    pump.reverse();
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

void init_device_id()
{
    init_prefs();
    pump.device_id = load_device_id();
}

void setup()
{
    // Configure system controls
    start_btn.attachClick(start);
    stop_btn.attachClick(stop);
    reverse_btn.attachClick(reverse);
    reverse_btn.attachLongPressStart(init_device_id);
    reverse_btn.setPressMs(5000);

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

void loop()
{
    start_btn.tick();
    stop_btn.tick();
    reverse_btn.tick();

    vTaskDelay(10);
}
