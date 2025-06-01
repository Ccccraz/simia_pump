#ifndef AT8236_HID
#define AT8236_HID
#include "config.h"

#include <Arduino.h>
#include <USBHID.h>

#include <atomic>
#include <cstring>

enum class set_feature_cmd_t : uint8_t
{
    SET_DEVICE_ID = 0x01,
    SET_WIFI = 0x02,
    SET_START_MODE = 0x03,
};

enum class get_feature_cmd_t : uint8_t
{
    GET_DEVICE_ID = 0x01,
    GET_WIFI = 0x02,
};

// HID Features
struct wifi_info_t
{
    uint8_t ssid_len;
    uint8_t password_len;
    uint8_t ssid[30];
    uint8_t password[30];
};

struct device_info_t
{
    uint8_t device_id;
    uint8_t nickname_len;
    uint8_t nickname[60];
};

union feature_payload_t
{
    wifi_info_t wifi_info;
    device_info_t device_info;
    simia::start_mode_t start_mode;
    uint8_t data[62];
};

#pragma pack(push, 1)
struct feature_t
{
    uint8_t device_id;
    feature_payload_t payload;
};
#pragma pack(pop)

// HID Report
enum output_cmd_t : uint8_t
{
    START = 0x00,
    STOP = 0x01,
    REVERSE = 0x02,
    SET_SPEED = 0x03,
};

#pragma pack(push, 1)
struct report_t
{
    uint8_t device_id;
    output_cmd_t cmd;
    uint32_t payload;
};
#pragma pack(pop)

// Events
ESP_EVENT_DECLARE_BASE(ARDUINO_USB_HID_SIMIA_PUMP_EVENTS);
enum arduino_usb_hid_simia_pump_event_t
{
    ARDUINO_USB_HID_SIMIA_PUMP_ANY_EVENT = ESP_EVENT_ANY_ID,
    ARDUINO_USB_HID_SIMIA_PUMP_SET_DEVICE_EVENT = 0,
    ARDUINO_USB_HID_SIMIA_PUMP_SET_WIFI_EVENT,
    ARDUINO_USB_HID_SIMIA_PUMP_SET_START_MODE_EVENT,
};

class AT8236HID : USBHIDDevice
{
private:
    // HID device
    char report_descriptor[115] = {
        0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
        0x09, 0x04,       // USAGE (Joystick)
        0xa1, 0x01,       // COLLECTION (Application)
        0xa1, 0x00,       //   COLLECTION (Physical)

        // Report ID 1
        0x85, 0x01,       //     REPORT_ID (1)
        0x05, 0x01,       //     USAGE_PAGE (Generic Desktop)
        0x09, 0x39,       //     USAGE (Hat switch)
        0x15, 0x01,       //     LOGICAL_MINIMUM (0)
        0x25, 0x08,       //     LOGICAL_MAXIMUM (8)
        0x75, 0x08,       //     REPORT_SIZE (8)
        0x95, 0x01,       //     REPORT_COUNT (1)
        0x81, 0x02,       //     INPUT (Data,Var,Abs)
        0x06, 0x00, 0xff, //     USAGE_PAGE (Vendor Defined Page 1)
        0x09, 0x01,       //     USAGE (Vendor Usage 1)
        0x75, 0x08,       //     REPORT_SIZE (8)
        0x95, 0x06,       //     REPORT_COUNT (6)
        0x91, 0x02,       //     OUTPUT (Data,Var,Abs)

        0x09, 0x02,       //     USAGE (Vendor Usage 2)
        0x75, 0x08,       //     REPORT_SIZE (8)
        0x95, 0x3F,       //     REPORT_COUNT (63)
        0xb1, 0x02,       //     FEATURE (Data,Var,Abs)

        // Report ID 2
        0x85, 0x02,       //     REPORT_ID (2)
        0x05, 0x01,       //     USAGE_PAGE (Generic Desktop)
        0x09, 0x39,       //     USAGE (Hat switch)
        0x15, 0x01,       //     LOGICAL_MINIMUM (0)
        0x25, 0x08,       //     LOGICAL_MAXIMUM (8)
        0x75, 0x08,       //     REPORT_SIZE (8)
        0x95, 0x01,       //     REPORT_COUNT (1)
        0x81, 0x02,       //     INPUT (Data,Var,Abs)
        0x06, 0x00, 0xff, //     USAGE_PAGE (Vendor Defined Page 1)
        0x09, 0x01,       //     USAGE (Vendor Usage 1)
        0x75, 0x08,       //     REPORT_SIZE (8)
        0x95, 0x06,       //     REPORT_COUNT (6)
        0x91, 0x02,       //     OUTPUT (Data,Var,Abs)

        0x09, 0x02,       //     USAGE (Vendor Usage 2)
        0x75, 0x08,       //     REPORT_SIZE (8)
        0x95, 0x3F,       //     REPORT_COUNT (63)
        0xb1, 0x02,       //     FEATURE (Data,Var,Abs)

        // Report ID 3
        0x85, 0x03,       //     REPORT_ID (3)
        0x05, 0x01,       //     USAGE_PAGE (Generic Desktop)
        0x09, 0x39,       //     USAGE (Hat switch)
        0x15, 0x01,       //     LOGICAL_MINIMUM (0)
        0x25, 0x08,       //     LOGICAL_MAXIMUM (8)
        0x75, 0x08,       //     REPORT_SIZE (8)
        0x95, 0x01,       //     REPORT_COUNT (1)
        0x81, 0x02,       //     INPUT (Data,Var,Abs)
        0x06, 0x00, 0xff, //     USAGE_PAGE (Vendor Defined Page 1)
        0x09, 0x01,       //     USAGE (Vendor Usage 1)
        0x75, 0x08,       //     REPORT_SIZE (8)
        0x95, 0x06,       //     REPORT_COUNT (6)
        0x91, 0x02,       //     OUTPUT (Data,Var,Abs)

        0x09, 0x02,       //     USAGE (Vendor Usage 2)
        0x75, 0x08,       //     REPORT_SIZE (8)
        0x95, 0x3F,       //     REPORT_COUNT (63)
        0xb1, 0x02,       //     FEATURE (Data,Var,Abs)

        0xc0,             //   END_COLLECTION
        0xc0              // END_COLLECTION
    };
    uint8_t _device_id{0};
    String _device_nickname{};

    simia::config_t &_config;

    // HID device
    USBHID _usbhid{};

    // Control pins
    uint8_t _in1_pin{};
    uint8_t _in2_pin{};

    // Speed control
    float _speed{};
    int _speed_to_report{};

    // Direction control
    int _direction{0};

    // Running control
    bool _rewarding{false};
    std::atomic<bool> stop_request_{false};

    QueueHandle_t _task_queue{};

    void _cmd_parser(report_t &report);
    static void _work_thread(void *param);
    void _start_direct();
    void _stop_direct();
    auto _start(uint32_t duration = 0) -> void;

    // Evnet handler
    void _on_set_device_id(const feature_t &feature);
    void _on_set_wifi(const feature_t &feature);
    void _on_set_start_mode(const feature_t &feature);

public:
    AT8236HID(uint8_t in1_pin, uint8_t in2_pin, float speed, simia::config_t &config);
    ~AT8236HID() = default;

    void add_task(uint32_t duration);
    auto stop(bool all = true) -> void;
    auto reverse() -> void;

    auto set_speed(uint32_t speed) -> void;
    auto set_device_info(uint8_t device_id, String device_nickname) -> void;

    auto begin() -> void;
    auto _onGetDescriptor(uint8_t *buffer) -> uint16_t;
    auto _onOutput(uint8_t report_id, const uint8_t *buffer, uint16_t len) -> void;
    auto _onSetFeature(uint8_t report_id, const uint8_t *buffer, uint16_t len) -> void;
    auto _onGetFeature(uint8_t report_id, uint8_t *buffer, uint16_t len) -> uint16_t;
    auto onEvent(esp_event_handler_t callback) -> void;
    auto onEvent(arduino_usb_hid_simia_pump_event_t event, esp_event_handler_t callback) -> void;
};

#endif