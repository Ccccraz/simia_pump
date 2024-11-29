#include "config.h"
#include <Arduino.h>
#include <USB.h>

// at8236 control pins
constexpr uint8_t first_pin{8};
constexpr uint8_t second_pin{18};

// system buttons
constexpr uint8_t start_button{40};
constexpr uint8_t stop_button{41};
constexpr uint8_t reverse_button{42};

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
    pinMode(start_button, INPUT_PULLUP);
    pinMode(stop_button, INPUT_PULLUP);
    pinMode(reverse_button, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(start_button), start, FALLING);
    attachInterrupt(digitalPinToInterrupt(stop_button), stop, FALLING);
    attachInterrupt(digitalPinToInterrupt(reverse_button), reverse, FALLING);

    // Configure USB
    USB.manufacturerName("simia");
    USB.productName("pump");
    USB.begin();
    pump.begin();
}

void loop()
{
}