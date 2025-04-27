#ifndef UTILS_H
#define UTILS_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

String parse_gitee_url(HTTPClient &http_client);

#endif // UTILS_H
