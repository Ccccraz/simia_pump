#ifndef AT8236_HID
#define AT8236_HID

#include <Arduino.h>
#include <USBHID.h>

#include <atomic>
#include <cstring>

ESP_EVENT_DECLARE_BASE(ARDUINO_USB_HID_SIMIA_PUMP_EVENTS);

enum arduino_usb_hid_simia_pump_event_t
{
    ARDUINO_USB_HID_SIMIA_PUMP_ANY_EVENT = ESP_EVENT_ANY_ID,
    ARDUINO_USB_HID_SIMIA_PUMP_SET_FEATURE_EVENT = 0,
    ARDUINO_USB_HID_SIMIA_PUMP_GET_FEATURE_EVENT,
    ARDUINO_USB_HID_SIMIA_PUMP_OUTPUT_EVENT,
    ARDUINO_USB_HID_SIMIA_PUMP_MAX_EVENT,
};

struct arduino_usb_hid_simia_pump_event_data_t
{
    const uint8_t *buffer;
    uint16_t len;
};

class AT8236HID : USBHIDDevice
{
  private:
    // HID device
    char report_descriptor[43] = {
        0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
        0x09, 0x04,       // USAGE (Joystick)
        0xa1, 0x01,       // COLLECTION (Application)
        0xa1, 0x00,       //   COLLECTION (Physical)
        0x05, 0x01,       //     USAGE_PAGE (Generic Desktop)
        0x09, 0x39,       //     USAGE (Hat switch)
        0x15, 0x01,       //     LOGICAL_MINIMUM (0)
        0x25, 0x08,       //     LOGICAL_MAXIMUM (8)
        0x75, 0x08,       //     REPORT_SIZE (8)
        0x95, 0x01,       //     REPORT_COUNT (1)
        0x81, 0x02,       //     INPUT (Data,Var,Abs)
        0x06, 0x00, 0xff, //     USAGE_PAGE (Vendor Defined Page 1)
        0x09, 0x01,       //     USAGE (Vendor Usage 1)
        0x75, 0x20,       //     REPORT_SIZE (32)
        0x95, 0x03,       //     REPORT_COUNT (3)
        0x91, 0x02,       //     OUTPUT (Data,Var,Abs)

        0x09, 0x02, //     USAGE (Vendor Usage 2)
        0x75, 0x08, //     REPORT_SIZE (8)
        0x95, 0x4B, //     REPORT_COUNT (75)
        0xb1, 0x02, //     FEATURE (Data,Var,Abs)

        0xc0, //   END_COLLECTION
        0xc0  // END_COLLECTION
    };

    enum cmd_t
    {
        START = 0,
        STOP = 1,
        REVERSE = 2,
        SET_SPEED = 3,
    };

    struct report_t
    {
        uint32_t device_id;
        uint32_t cmd;
        uint32_t payload;
    };

    // TODO: Preset SSID and password
    struct feature_t
    {
        uint32_t device_id;
        uint32_t new_device_id;

        struct
        {
            uint8_t ssid_len;
            uint8_t password_len;
            char ssid[32];
            char password[32];
            char need_wifi;
        } wifi;
    };

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

  public:
    uint32_t device_id{0};

    AT8236HID(uint8_t in1_pin, uint8_t in2_pin, float speed);
    ~AT8236HID() = default;

    void add_task(uint32_t duration);
    auto stop(bool all = true) -> void;
    auto reverse() -> void;

    auto set_speed(uint32_t speed) -> void;

    auto begin() -> void;
    auto _onGetDescriptor(uint8_t *buffer) -> uint16_t;
    auto _onOutput(uint8_t report_id, const uint8_t *buffer, uint16_t len) -> void;
    auto _onSetFeature(uint8_t report_id, const uint8_t *buffer, uint16_t len) -> void;
    auto _onGetFeature(uint8_t report_id, uint8_t *buffer, uint16_t len) -> uint16_t;
    auto onEvent(esp_event_handler_t callback) -> void;
    auto onEvent(arduino_usb_hid_simia_pump_event_t event, esp_event_handler_t callback) -> void;
};

#endif