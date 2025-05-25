#ifndef ALERT_H
#define ALERT_H

#include <Arduino.h>
#include <config.h>

namespace alert
{
void running();

void start_ota();

void success();

void failed();

void _start_pump_positive();

void _start_pump_negative();

void _stop_pump();
} // namespace alert

#endif // ALERT_H