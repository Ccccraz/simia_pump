#include "at8236_hid.h"

ESP_EVENT_DEFINE_BASE(ARDUINO_USB_HID_SIMIA_PUMP_EVENTS);
esp_err_t arduino_usb_event_post(esp_event_base_t event_base, int32_t event_id, void *event_data,
                                 size_t event_data_size, TickType_t ticks_to_wait);
esp_err_t arduino_usb_event_handler_register_with(esp_event_base_t event_base, int32_t event_id,
                                                  esp_event_handler_t event_handler, void *event_handler_arg);

void AT8236HID::_cmd_parser(report_t &report)
{
    if (report.device_id != device_id)
        return;

    switch (report.cmd)
    {
    case START:
        add_task(report.payload);
        break;
    case STOP:
        if (report.payload != 0)
        {
            stop(false);
        }
        else
        {
            stop(true);
        }
        break;
    case REVERSE:
        reverse();
        break;
    case SET_SPEED:
        set_speed(report.payload);
        break;
    default:
        break;
    }
}

void AT8236HID::_work_thread(void *param)
{
    AT8236HID *pump = static_cast<AT8236HID *>(param);
    uint32_t duration{};
    while (true)
    {
        if (xQueueReceive(pump->_task_queue, &duration, portMAX_DELAY) == pdPASS)
        {
            pump->_start(duration);
        }
    }
}

void AT8236HID::add_task(uint32_t duration)
{
    xQueueSend(_task_queue, &duration, 0);
}

void AT8236HID::_start_direct()
{
    _stop_direct();

    analogWrite(_in1_pin, _speed_to_report);
    analogWrite(_in2_pin, LOW);

    _rewarding = true;
}

void AT8236HID::_stop_direct()
{
    analogWrite(_in1_pin, LOW);
    analogWrite(_in2_pin, LOW);

    _rewarding = false;
}

/// @brief Init AT8236Brushless
/// @param in1_pin Positive pin
/// @param in2_pin Negative pin
/// @param speed Initial speed
AT8236HID::AT8236HID(uint8_t in1_pin, uint8_t in2_pin, float speed)
    : _in1_pin(in1_pin), _in2_pin(in2_pin), _speed(constrain(speed, 0.0f, 1.0f))
{
    // Set up positive pin and negative pin
    pinMode(_in1_pin, OUTPUT);
    pinMode(_in2_pin, OUTPUT);
    _speed_to_report = static_cast<int>(speed * 255);

    // Set up HID device
    static bool initialized{false};
    if (!initialized)
    {
        initialized = true;
        _usbhid.addDevice(this, sizeof(report_descriptor));
    }

    // Create task queue
    _task_queue = xQueueCreate(100, sizeof(uint32_t));

    this->_stop_direct();
}

/// @brief Start the pump with a given duration
/// @param duraiton Duration of the pump
/// @return
auto AT8236HID::_start(uint32_t duration) -> void
{
    analogWrite(_in1_pin, _speed_to_report);
    analogWrite(_in2_pin, LOW);

    _rewarding = true;

    if (duration == 0)
    {
        while (true)
        {
            constexpr uint32_t check_interval_ms = 100;
            vTaskDelay(pdMS_TO_TICKS(check_interval_ms));

            if (stop_request_.load())
            {
                stop_request_.store(false);
                break;
            }
        }
    }
    else
    {
        const uint32_t start_time = millis();
        const uint32_t end_time = start_time + duration;
        while (millis() < end_time)
        {
            constexpr uint32_t check_interval_ms = 100;
            const uint32_t remaining_time = end_time - millis();
            const uint32_t delay_time = std::min(check_interval_ms, remaining_time);

            vTaskDelay(pdMS_TO_TICKS(delay_time));

            if (stop_request_.load())
            {
                stop_request_.store(false);
                break;
            }
        }
    }
    _stop_direct();
}

auto AT8236HID::stop(bool all) -> void
{
    analogWrite(_in1_pin, LOW);
    analogWrite(_in2_pin, LOW);

    if (all)
    {
        uint32_t receivedItem{};
        while (xQueueReceive(_task_queue, &receivedItem, 0) == pdTRUE)
        {
        }
    }

    if (_rewarding)
    {
        _rewarding = false;
        stop_request_.store(true);
    }
}

auto AT8236HID::reverse() -> void
{
    if (_rewarding)
    {
        _stop_direct();
        std::swap(_in1_pin, _in2_pin);
        _start_direct();
    }
    else
    {
        std::swap(_in1_pin, _in2_pin);
    }
}

auto AT8236HID::begin() -> void
{
    _usbhid.begin();

    TaskHandle_t worker_task_handle{};
    xTaskCreate(_work_thread, "AT8236HID", 1024 * 8, this, configMAX_PRIORITIES - 1, &worker_task_handle);
}

auto AT8236HID::_onGetDescriptor(uint8_t *buffer) -> uint16_t
{
    memcpy(buffer, report_descriptor, sizeof(report_descriptor));
    return sizeof(report_descriptor);
}

auto AT8236HID::_onOutput(uint8_t report_id, const uint8_t *buffer, uint16_t len) -> void
{
    report_t report{};
    memcpy(&report, buffer, sizeof(report));
    _cmd_parser(report);
}

auto AT8236HID::_onSetFeature(uint8_t report_id, const uint8_t *buffer, uint16_t len) -> void
{
    feature_t feature{};
    memcpy(&feature, buffer, sizeof(feature));

    if (feature.device_id != device_id)
        return;

    arduino_usb_hid_simia_pump_event_data_t event_data{};
    event_data.buffer = buffer;
    event_data.len = len;

    switch (feature.cmd)
    {
    case SET_DEVICE_ID:
        // this->device_id = feature.new_device_id;
        // arduino_usb_event_post(ARDUINO_USB_HID_SIMIA_PUMP_EVENTS, ARDUINO_USB_HID_SIMIA_PUMP_SET_DEVICE_EVENT,
        //                        &event_data, sizeof(arduino_usb_hid_simia_pump_event_data_t), portMAX_DELAY);
        break;
    case SET_WIFI:
        // this->ssid = feature.wifi.ssid;
        // this->password = feature.wifi.password;
        // this->need_wifi = feature.wifi.need_wifi;
        // arduino_usb_event_post(ARDUINO_USB_HID_SIMIA_PUMP_EVENTS, ARDUINO_USB_HID_SIMIA_PUMP_SET_WIFI_EVENT,
        //                        &event_data, sizeof(arduino_usb_hid_simia_pump_event_data_t), portMAX_DELAY);
        break;
    case SET_OTA:
        // if (feature.ota.need_ota == 0x01)
        // {
        arduino_usb_event_post(ARDUINO_USB_HID_SIMIA_PUMP_EVENTS, ARDUINO_USB_HID_SIMIA_PUMP_SET_OTA_EVENT, &event_data,
                               sizeof(arduino_usb_hid_simia_pump_event_data_t), portMAX_DELAY);
        // }
        break;
    default:
        break;
    }
}

auto AT8236HID::_onGetFeature(uint8_t report_id, uint8_t *buffer, uint16_t len) -> uint16_t
{
    feature_t fea{};
    fea.device_id = device_id;
    fea.cmd = SET_DEVICE_ID;
    // fea.new_device_id = device_id;

    // fea.wifi.ssid_len = this->ssid.length();
    // memcpy(fea.wifi.ssid, this->ssid.c_str(), fea.wifi.ssid_len);
    // fea.wifi.password_len = this->password.length();
    // memcpy(fea.wifi.password, this->password.c_str(), fea.wifi.password_len);

    // fea.ota.need_ota = 0x00;

    memcpy(buffer, &fea, len);

    return len;
}

auto AT8236HID::onEvent(esp_event_handler_t callback) -> void
{
    this->onEvent(ARDUINO_USB_HID_SIMIA_PUMP_ANY_EVENT, callback);
}

auto AT8236HID::onEvent(arduino_usb_hid_simia_pump_event_t event, esp_event_handler_t callback) -> void
{
    arduino_usb_event_handler_register_with(ARDUINO_USB_HID_SIMIA_PUMP_EVENTS, event, callback, this);
}

auto AT8236HID::set_speed(uint32_t speed) -> void
{
    _speed = constrain(speed, 0, 100) / 100.0f;
    _speed_to_report = static_cast<int>(_speed * 255);

    if (_rewarding)
    {
        analogWrite(_in1_pin, _speed_to_report);
    }
}