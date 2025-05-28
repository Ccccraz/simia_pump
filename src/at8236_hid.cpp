#include "at8236_hid.h"
#include "config.h"

ESP_EVENT_DEFINE_BASE(ARDUINO_USB_HID_SIMIA_PUMP_EVENTS);
esp_err_t arduino_usb_event_post(esp_event_base_t event_base, int32_t event_id, void *event_data,
                                 size_t event_data_size, TickType_t ticks_to_wait);
esp_err_t arduino_usb_event_handler_register_with(esp_event_base_t event_base, int32_t event_id,
                                                  esp_event_handler_t event_handler, void *event_handler_arg);

void AT8236HID::_cmd_parser(report_t &report)
{
    if (report.device_id != this->_device_id)
        return;

    switch (report.cmd)
    {
    case output_cmd_t::START:
        add_task(report.payload);
        break;
    case output_cmd_t::STOP:
        if (report.payload != 0)
        {
            stop(false);
        }
        else
        {
            stop(true);
        }
        break;
    case output_cmd_t::REVERSE:
        reverse();
        break;
    case output_cmd_t::SET_SPEED:
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

void AT8236HID::_on_set_device_id(const feature_t &feature)
{
    this->_device_id = feature.payload.device_info.device_id;
    this->_device_nickname = String(feature.payload.device_info.nickname, feature.payload.device_info.nickname_len);
    auto data = feature.payload.device_info;
    arduino_usb_event_post(ARDUINO_USB_HID_SIMIA_PUMP_EVENTS, ARDUINO_USB_HID_SIMIA_PUMP_SET_DEVICE_EVENT, &data,
                           sizeof(device_info_t), portMAX_DELAY);
}

void AT8236HID::_on_set_wifi(const feature_t &feature)
{
    wifi_info_t wifi_info{};
    wifi_info.ssid_len = feature.payload.wifi_info.ssid_len;
    wifi_info.password_len = feature.payload.wifi_info.password_len;
    memcpy(wifi_info.ssid, feature.payload.wifi_info.ssid, wifi_info.ssid_len);
    memcpy(wifi_info.password, feature.payload.wifi_info.password, wifi_info.password_len);
    arduino_usb_event_post(ARDUINO_USB_HID_SIMIA_PUMP_EVENTS, ARDUINO_USB_HID_SIMIA_PUMP_SET_WIFI_EVENT, &wifi_info,
                           sizeof(wifi_info_t), portMAX_DELAY);
}

void AT8236HID::_on_set_start_mode(const feature_t &feature)
{
    auto data = feature.payload.start_mode;
    arduino_usb_event_post(ARDUINO_USB_HID_SIMIA_PUMP_EVENTS, ARDUINO_USB_HID_SIMIA_PUMP_SET_START_MODE_EVENT, &data,
                           sizeof(simia::start_mode_t), portMAX_DELAY);
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

    xTaskCreatePinnedToCore(_work_thread, "AT8236HID", 1024 * 8, this, configMAX_PRIORITIES - 1, nullptr, 1);
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
    // detect set feature command
    auto cmd = static_cast<set_feature_cmd_t>(report_id);

    // parse feature data
    feature_t feature{};
    memcpy(&feature, buffer, len);

    // check device id
    if (feature.device_id != this->_device_id)
        return;

    // dispatch command
    switch (cmd)
    {
    case set_feature_cmd_t::SET_DEVICE_ID:
        this->_on_set_device_id(feature);
        break;

    case set_feature_cmd_t::SET_WIFI:
        this->_on_set_wifi(feature);
        break;

    case set_feature_cmd_t::SET_START_MODE:
        this->_on_set_start_mode(feature);
        break;

    default:
        break;
    }
}

auto AT8236HID::_onGetFeature(uint8_t report_id, uint8_t *buffer, uint16_t len) -> uint16_t
{
    // prepare feature data
    feature_t fea{};

    // detect get feature command
    auto cmd = static_cast<get_feature_cmd_t>(report_id);

    // load config
    auto config = simia::load_config();

    switch (cmd)
    {
    case get_feature_cmd_t::GET_DEVICE_ID:
        fea.device_id = this->_device_id;
        fea.payload.device_info.device_id = this->_device_id;
        fea.payload.device_info.nickname_len = this->_device_nickname.length();
        memcpy(fea.payload.device_info.nickname, this->_device_nickname.c_str(), fea.payload.device_info.nickname_len);
        break;

    case get_feature_cmd_t::GET_WIFI:
        fea.device_id = this->_device_id;
        fea.payload.wifi_info.ssid_len = config.wifi.ssid.length();
        fea.payload.wifi_info.password_len = config.wifi.password.length();
        memcpy(fea.payload.wifi_info.ssid, config.wifi.ssid.c_str(), fea.payload.wifi_info.ssid_len);
        memcpy(fea.payload.wifi_info.password, config.wifi.password.c_str(), fea.payload.wifi_info.password_len);
        break;

    default:
        break;
    }

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

auto AT8236HID::set_device_id(uint8_t device_id) -> void
{
    this->_device_id = device_id;
}
