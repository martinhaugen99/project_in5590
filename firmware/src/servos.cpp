#include "servos.h"

#include <Servo.h>

#include "config.h"
#include "pins.h"
#include "safety.h"

namespace {

constexpr uint8_t SERVO_PINS[2] = {pins::BRUSH_SERVO, pins::BUCKET_SERVO};
Servo servos_[2];
uint16_t pulse_us_[2] = {0, 0};

size_t index_of(ServoId id) {
    return static_cast<size_t>(id);
}

}  // namespace

namespace servos {

void begin() {
    // Hold the signal lines low until the first pulse, so they do not float.
    for (const uint8_t pin : SERVO_PINS) {
        digitalWrite(pin, LOW);
        pinMode(pin, OUTPUT);
    }
}

uint16_t set_pulse_us(ServoId id, uint16_t pulse_us) {
    const size_t i = index_of(id);
    if (id == ServoId::BUCKET && !safety::bucket_may_move()) {
        return 0;
    }
    const uint16_t clamped = constrain(pulse_us, config::SERVO_MIN_US, config::SERVO_MAX_US);
    if (!servos_[i].attached()) {
        // attach() starts pulsing at once with the stored width, which the constructor sets
        // to 1500 us (Servo.cpp). Store the new width first, so the first pulse is already the
        // right one. Before attach(), writeMicroseconds() clamps to 544-2400 us.
        servos_[i].writeMicroseconds(clamped);
        servos_[i].attach(SERVO_PINS[i], config::SERVO_MIN_US, config::SERVO_MAX_US);
    }
    servos_[i].writeMicroseconds(clamped);
    pulse_us_[i] = clamped;
    return clamped;
}

void release(ServoId id) {
    const size_t i = index_of(id);
    servos_[i].detach();
    pulse_us_[i] = 0;
}

uint16_t pulse_us(ServoId id) {
    return pulse_us_[index_of(id)];
}

}  // namespace servos
