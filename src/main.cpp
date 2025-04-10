#include "config.h"
#include <Arduino.h>
#include <USB.h>
#include <OneButton.h>

// at8236 control pins
constexpr uint8_t first_pin{8};
constexpr uint8_t second_pin{18};

// system buttons
constexpr uint8_t start_pin{35};
constexpr uint8_t stop_pin{36};
constexpr uint8_t reverse_pin{37};

OneButton start_btn(start_pin, true);
OneButton stop_btn(stop_pin, true);
OneButton reverse_btn(reverse_pin, true);


#if MODE == 0

// #include "at8236_brushless_hid.h"
#include "at8236_hid.h"

// simia::AT8236BrushlessHID pump(first_pin, second_pin, 0, speed_ctrl_pin, report_pin, direction_ctrl_pin);
simia::AT8236HID pump(first_pin, second_pin, 1.0f);

#endif

void start()
{
    pump.start();
}

void stop()
{
    pump.stop();
}

void reverse()
{
    pump.reverse();
}

void setup()
{
    // Configure system controls
    start_btn.attachClick(start);
    stop_btn.attachClick(stop);
    reverse_btn.attachClick(reverse);

    // Configure USB
    USB.manufacturerName("simia");
    USB.productName("pump");
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