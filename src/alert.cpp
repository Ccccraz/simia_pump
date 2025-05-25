#include "alert.h"
namespace alert
{
void _start_pump_positive()
{
    gpio_set_direction(first_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(second_pin, GPIO_MODE_OUTPUT);

    gpio_set_level(first_pin, HIGH);
    gpio_set_level(second_pin, LOW);
}

void _start_pump_negative()
{
    gpio_set_direction(first_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(second_pin, GPIO_MODE_OUTPUT);

    gpio_set_level(first_pin, LOW);
    gpio_set_level(second_pin, HIGH);
}

void _stop_pump()
{
    gpio_set_level(first_pin, LOW);
    gpio_set_level(second_pin, LOW);
}
void running()
{
    _stop_pump();
    _start_pump_positive();
}

void start_ota()
{
    _start_pump_positive();

    vTaskDelay(pdMS_TO_TICKS(1000));

    _stop_pump();
}

void success()
{
    for (size_t i = 0; i < 10; i++)
    {
        _stop_pump();
        _start_pump_negative();

        vTaskDelay(pdMS_TO_TICKS(500));
        _stop_pump();

        _start_pump_negative();

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void failed()
{
    for (size_t i = 0; i < 2; i++)
    {
        _stop_pump();
        _start_pump_positive();

        vTaskDelay(pdMS_TO_TICKS(2000));
        _stop_pump();

        _start_pump_positive();

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

} // namespace alert
