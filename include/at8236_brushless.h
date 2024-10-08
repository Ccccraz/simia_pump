#ifndef AT8236_BRUSHLESS_H
#define AT8236_BRUSHLESS_H
#include <Arduino.h>

namespace simia
{
class AT8236Brushless
{
  private:
    // Control pins
    uint8_t _in1_pin{};
    uint8_t _in2_pin{};

    // Speed control
    uint8_t _speed_ctrl_pin{};
    uint8_t _report_pin{};
    float _speed{};
    int _speed_to_report{};

    // Direction control
    uint8_t _direction_ctrl_pin{};
    unsigned long _last_reverse_time;
    int _direction{0};

    // Running control
    bool _rewarding{false};
    bool _running{false};

  public:
    AT8236Brushless(uint8_t in1_pin, uint8_t in2_pin, float speed, uint8_t speed_ctrl_pin, uint8_t report_pin,
                    uint8_t direction_ctrl_pin);
    ~AT8236Brushless() = default;

    auto start(int duration = -1) -> void;

    auto stop() -> void;

    auto reverse() -> void;

    auto set_speed(float speed) -> void;
    auto get_speed() -> float;
};

/// @brief Init AT8236Brushless
/// @param in1_pin Positive pin
/// @param in2_pin Negative pin
/// @param speed Initial speed
/// @param speed_ctrl_pin Speed control pin
/// @param report_pin Report pin
/// @param direction_ctrl_pin Running direction control pin
inline AT8236Brushless::AT8236Brushless(uint8_t in1_pin, uint8_t in2_pin, float speed, uint8_t speed_ctrl_pin,
                                        uint8_t report_pin, uint8_t direction_ctrl_pin)
    : _in1_pin(in1_pin), _in2_pin(in2_pin), _speed(constrain(speed, 0.0f, 1.0f)), _speed_ctrl_pin(speed_ctrl_pin),
      _report_pin(report_pin), _direction_ctrl_pin(direction_ctrl_pin)
{
    // Set up positive pin and negative pin
    pinMode(_in1_pin, OUTPUT);
    pinMode(_in2_pin, OUTPUT);

    // Set up speed control pin
    pinMode(_speed_ctrl_pin, OUTPUT);
    // Set up report pin
    pinMode(_report_pin, INPUT_PULLDOWN);
    _speed_to_report = static_cast<int>(speed * 255);

    // Set up direction control pin
    pinMode(_direction_ctrl_pin, OUTPUT);
    digitalWrite(_direction_ctrl_pin, LOW);
    // Anti-shake
    _last_reverse_time = millis();

    this->stop();
}

/// @brief Start the pump with a given duration
/// @param duraiton Duration of the pump
/// @return
inline auto AT8236Brushless::start(int duraiton) -> void
{
    this->stop();

    digitalWrite(_in1_pin, LOW);
    digitalWrite(_in2_pin, HIGH);

    analogWrite(_speed_ctrl_pin, _speed_to_report);

    _rewarding = true;

    if (duraiton > -1)
    {
        vTaskDelay(duraiton);
        this->stop();
    }
}

inline auto AT8236Brushless::stop() -> void
{
    digitalWrite(_in1_pin, LOW);
    digitalWrite(_in2_pin, LOW);
    analogWrite(_speed_ctrl_pin, LOW);

    _rewarding = false;
}

inline auto AT8236Brushless::reverse() -> void
{
    // Get current time for anti-shake
    auto currecnt_time = millis();

    if (currecnt_time - _last_reverse_time > 1000)
    {
        switch (_direction)
        {
        case 0:
            digitalWrite(_direction_ctrl_pin, HIGH);
            _direction = 1;
            break;
        case 1:
            digitalWrite(_direction_ctrl_pin, LOW);
            _direction = 0;
            break;
        default:
            break;
        }

        // Update last reverse time
        _last_reverse_time = millis();
    }
}

inline auto AT8236Brushless::get_speed() -> float
{
    return analogRead(_report_pin);
}

inline auto AT8236Brushless::set_speed(float speed) -> void
{
    _speed = constrain(speed, 0.0f, 1.0f);
    _speed = 1.0f - _speed;
    _speed_to_report = static_cast<int>(_speed * 255);

    if (_rewarding)
    {
        Serial.println(_speed_to_report);
        analogWrite(_speed_ctrl_pin, _speed_to_report);
    }
}

} // namespace simia

#endif