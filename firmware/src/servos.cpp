#include "servos.h"

#include "config.h"
#include "pins.h"

namespace {

constexpr uint8_t SERVO_PINS[2] = {pins::BRUSH_SERVO, pins::BUCKET_SERVO};
uint32_t pulse_us_[2] = {0, 0};

size_t index_of(ServoId id) {
    return static_cast<size_t>(id);
}

uint32_t duty_for(uint32_t pulse_us) {
    return static_cast<uint32_t>((static_cast<uint64_t>(pulse_us) << config::SERVO_PWM_BITS) /
                                 config::SERVO_PERIOD_US);
}

}  // namespace

namespace servos {

bool begin() {
    bool ok = true;
    for (const uint8_t pin : SERVO_PINS) {
        ok = ledcAttach(pin, config::SERVO_PWM_HZ, config::SERVO_PWM_BITS) && ok;
        ledcWrite(pin, 0);
    }
    return ok;
}

uint32_t set_pulse_us(ServoId id, uint32_t pulse_us) {
    const size_t i = index_of(id);
    const uint32_t clamped = constrain(pulse_us, config::SERVO_MIN_US, config::SERVO_MAX_US);
    pulse_us_[i] = clamped;
    ledcWrite(SERVO_PINS[i], duty_for(clamped));
    return clamped;
}

void release(ServoId id) {
    const size_t i = index_of(id);
    pulse_us_[i] = 0;
    ledcWrite(SERVO_PINS[i], 0);
}

uint32_t pulse_us(ServoId id) {
    return pulse_us_[index_of(id)];
}

}  // namespace servos
