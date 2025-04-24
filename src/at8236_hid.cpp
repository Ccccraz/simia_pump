#include "at8236_hid.h"

namespace simia
{
void AT8236HID::cmd_parser(report_t &report)
{
    if (report.device_id != device_id)
        return;

    switch (report.cmd)
    {
    case START:
        add_task(report.payload);
        break;
    case STOP:
        stop();
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

void AT8236HID::work_thread(void *param)
{
    AT8236HID *pump = static_cast<AT8236HID *>(param);
    uint32_t duration{};
    while (true)
    {
        if (xQueueReceive(pump->task_queue, &duration, portMAX_DELAY) == pdPASS)
        {
            pump->start(duration);
        }
    }
}

void AT8236HID::add_task(uint32_t duration)
{
    xQueueSend(task_queue, &duration, 0);
}

void AT8236HID::start_direct()
{
    stop_direct();

    analogWrite(_in1_pin, _speed_to_report);
    analogWrite(_in2_pin, LOW);

    _rewarding = true;
}

void AT8236HID::stop_direct()
{
    analogWrite(_in1_pin, LOW);
    analogWrite(_in2_pin, LOW);

    _rewarding = false;
}

/// @brief Init AT8236Brushless
/// @param in1_pin Positive pin
/// @param in2_pin Negative pin
/// @param speed Initial speed
AT8236HID::AT8236HID(uint8_t in1_pin, uint8_t in2_pin, float speed, uint32_t device_id)
    : _in1_pin(in1_pin), _in2_pin(in2_pin), _speed(constrain(speed, 0.0f, 1.0f)), device_id(device_id)
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
    task_queue = xQueueCreate(100, sizeof(uint32_t));

    this->stop_direct();
}

AT8236HID::~AT8236HID()
{
}

/// @brief Start the pump with a given duration
/// @param duraiton Duration of the pump
/// @return
auto AT8236HID::start(uint32_t duration) -> void
{
    analogWrite(_in1_pin, _speed_to_report);
    analogWrite(_in2_pin, LOW);

    _rewarding = true;

    const uint32_t start_time = millis();
    const uint32_t end_time = start_time + duration;

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
    stop_direct();
}

auto AT8236HID::stop() -> void
{
    analogWrite(_in1_pin, LOW);
    analogWrite(_in2_pin, LOW);

    uint32_t receivedItem{};
    while (xQueueReceive(task_queue, &receivedItem, 0) == pdTRUE)
    {
    }

    _rewarding = false;
    stop_request_.store(true);
}

auto AT8236HID::reverse() -> void
{
    if (_rewarding)
    {
        stop_direct();
        std::swap(_in1_pin, _in2_pin);
        start_direct();
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
    xTaskCreate(work_thread, "AT8236HID", 1024 * 8, this, tskIDLE_PRIORITY + 1, &worker_task_handle);
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
    cmd_parser(report);
}

auto AT8236HID::_onSetFeature(uint8_t report_id, const uint8_t *buffer, uint16_t len) -> void
{
    feature_t feature{};
    memcpy(&feature, buffer, sizeof(feature));

    if (feature.device_id != device_id)
        return;

    device_id = feature.new_device_id;
}

auto AT8236HID::_onGetFeature(uint8_t report_id, uint8_t *buffer, uint16_t len) -> uint16_t
{
    uint32_t feature[]{device_id, device_id};

    memcpy(buffer, &feature, sizeof(feature));

    return sizeof(feature);
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

auto AT8236HID::get_speed() -> float
{
    return _speed;
}

} // namespace simia
