#include "config.h"
#include <Arduino.h>
#include <USB.h>

// at8236 control pins
constexpr uint8_t first_pin{1};
constexpr uint8_t second_pin{2};

// system buttons
constexpr uint8_t start_button{7};
constexpr uint8_t stop_button{8};
constexpr uint8_t reverse_button{9};
constexpr uint8_t speed_button{10};

#if MODE == 0

#include "at8236_brushless_hid.h"

// komoer brushless motor control pins
constexpr uint8_t speed_ctrl_pin{3};
constexpr uint8_t direction_ctrl_pin{4};
constexpr uint8_t report_pin{5};
float speed{0.0f};

simia::AT8236BrushlessHID pump(first_pin, second_pin, 0, speed_ctrl_pin, report_pin, direction_ctrl_pin);

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
    pinMode(speed_button, INPUT_PULLDOWN);

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
    // Speed control
    auto val = analogRead(speed_button);
    if (val - 50 > speed || val + 50 < speed)
    {
        speed = val;
        pump.set_speed(val / 4095.0f);
    }

    delay(10);
}