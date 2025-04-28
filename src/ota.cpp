#include "ota.h"

void ota_update(PubSubClient &mqtt_client)
{
    alert::running();
    mqtt_client.publish(mqtt_topic_pub, "Start parse url");

    auto result = parse_gitee_url();

    if (result.code == 200)
    {
        mqtt_client.publish(mqtt_topic_pub, result.data.c_str());
        alert::success();
    }
    else
    {
        std::string error = "Error code: " + std::to_string(result.code);
        mqtt_client.publish(mqtt_topic_pub, error.c_str());
        alert::failed();
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        esp_http_client_config_t config = {
            .url = result.data.c_str(),
            .cert_pem = ca_cert,
        };

        esp_https_ota_config_t ota_config = {
            .http_config = &config,
        };

        mqtt_client.publish(mqtt_topic_pub, "Start OTA update");

        esp_err_t ret = esp_https_ota(&ota_config);
        if (ret == ESP_OK)
        {
            mqtt_client.publish(mqtt_topic_pub, "OTA update success");
            alert::success();

            esp_restart();
        }
        else
        {
            mqtt_client.publish(mqtt_topic_pub, "OTA update failed");
            alert::failed();

            esp_restart();
        }
    }
}

void ota_without_mqtt()
{
    alert::running();
    auto result = parse_gitee_url();

    if (result.code != 200)
    {
        alert::failed();
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        esp_http_client_config_t config = {
            .url = result.data.c_str(),
            .cert_pem = ca_cert,
        };

        esp_https_ota_config_t ota_config = {
            .http_config = &config,
        };

        esp_err_t ret = esp_https_ota(&ota_config);
        if (ret == ESP_OK)
        {
            alert::success();
            esp_restart();
        }
        else
        {
            alert::failed();
            esp_restart();
        }
    }
}

parse_result_t parse_gitee_url()
{
    const char *giteeUrl = "https://gitee.com/api/v5/repos/ccccraz/simia_pump/releases/latest";

    WiFiClientSecure client{};
    HTTPClient http_client{};

    client.setCACert(ca_cert);

    parse_result_t result{
        .code = 200,
        .data = "",
    };

    http_client.begin(client, giteeUrl);
    auto code = http_client.GET();

    if (code != 200)
    {
        result.code = code;
        return result;
    }

    http_client.end();

    JsonDocument doc{};
    auto error = deserializeJson(doc, http_client.getString());

    JsonArray assets = doc["assets"];

    for (JsonVariant v : assets)
    {
        if (v["name"] == "firmware.bin")
        {
            result.data = v["browser_download_url"].as<String>();
            break;
        }
    }
    return result;
}