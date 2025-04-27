#include "utils.h"

String parse_gitee_url(HTTPClient &http_client)
{
    const char *giteeUrl = "https://gitee.com/api/v5/repos/ccccraz/simia_pump/releases/latest";

    String url = "";
    http_client.begin(giteeUrl);
    auto code = http_client.GET();

    if (code != 200)
    {
        return url;
    }

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