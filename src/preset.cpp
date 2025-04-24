#include "preset.h"
#include "Preferences.h"

Preferences prefs{};

const char* pref_name = "simia_pump_A100";
const char* pref_key = "device_id";

void init_prefs()
{
    prefs.begin(pref_name, false);
    prefs.putUInt(pref_key, 0);
    prefs.end();
}

uint32_t load_device_id()
{
    prefs.begin(pref_name, true);

    if (!prefs.isKey(pref_key))
    {
        prefs.end();
        init_prefs();
        prefs.begin(pref_name, true);
    }

    uint32_t id = prefs.getUInt(pref_key, 0);
    prefs.end();
    return id;
}

void save_device_id(uint32_t id){
    prefs.begin(pref_name, false);
    prefs.putUInt(pref_key, id);
    prefs.end();
}