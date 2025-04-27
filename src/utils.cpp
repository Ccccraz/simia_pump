#include "utils.h"
#include "mqtt.h"

String parse_gitee_url()
{
    const char *giteeUrl = "https://gitee.com/api/v5/repos/ccccraz/simia_pump/releases/latest";

    WiFiClientSecure client{};
    HTTPClient http_client{};

    client.setCACert(ca_cert);

    String url = "";
    http_client.begin(client, giteeUrl);
    auto code = http_client.GET();

    if (code != 200)
    {
        url = String(code);
        return url;
    }

    http_client.end();

    JsonDocument doc{};
    auto error = deserializeJson(doc, http_client.getString());

    JsonArray assets = doc["assets"];

    for (JsonVariant v : assets)
    {
        if (v["name"] == "firmware.bin")
        {
            url = v["browser_download_url"].as<String>();
        }
    }
    return url;
}