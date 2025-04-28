#ifndef OTA_H
#define OTA_H

#include "mqtt.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_https_ota.h>
#include "alert.h"

struct parse_result_t
{
    int code;
    String data;
};

parse_result_t parse_gitee_url();

void ota_update(PubSubClient &mqtt_client);
void ota_without_mqtt();

#endif // OTA_H