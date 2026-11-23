#include "actuator.h"

#include "config.h"
#include "pins.h"
#include "safety.h"

namespace {

actuator::Motion motion_ = actuator::Motion::STOP;
uint32_t stop_at_ms_ = 0;

// The PQ12 extends with one polarity on motor pins 2 and 3 and retracts with the other
// (PQ12 datasheet). Which driver input gives "extend" depends on the wiring and is set in
// config::ACT_EXTEND_ON_IN1. D8 and D10 have no PWM (D020), so the actuator runs at full speed.
void write_motion(actuator::Motion motion) {
    const uint8_t extend_pin = config::ACT_EXTEND_ON_IN1 ? pins::ACT_IN1 : pins::ACT_IN2;
    const uint8_t retract_pin = config::ACT_EXTEND_ON_IN1 ? pins::ACT_IN2 : pins::ACT_IN1;
    switch (motion) {
        case actuator::Motion::EXTEND:
            digitalWrite(retract_pin, LOW);
            digitalWrite(extend_pin, HIGH);
            break;
        case actuator::Motion::RETRACT:
            digitalWrite(extend_pin, LOW);
            digitalWrite(retract_pin, HIGH);
            break;
        case actuator::Motion::STOP:
            digitalWrite(extend_pin, LOW);
            digitalWrite(retract_pin, LOW);
            break;
    }
}

}  // namespace

namespace actuator {

void begin() {
    digitalWrite(pins::ACT_IN1, LOW);
    digitalWrite(pins::ACT_IN2, LOW);
    pinMode(pins::ACT_IN1, OUTPUT);
    pinMode(pins::ACT_IN2, OUTPUT);
    motion_ = Motion::STOP;
}

uint32_t jog(Motion motion, uint32_t run_ms) {
    if (motion != Motion::STOP && !safety::actuator_may_move()) {
        return 0;
    }
    const uint32_t used_ms = min(run_ms, config::ACT_MAX_JOG_MS);
    motion_ = motion;
    stop_at_ms_ = millis() + used_ms;
    write_motion(motion);
    safety::kick_watchdog();  // arms the watchdog at once when the actuator starts
    return used_ms;
}

void stop() {
    motion_ = Motion::STOP;
    write_motion(Motion::STOP);
}

void update(uint32_t now_ms) {
    if (motion_ != Motion::STOP && static_cast<int32_t>(now_ms - stop_at_ms_) >= 0) {
        stop();
    }
}

Motion motion() {
    return motion_;
}

uint16_t feedback_raw() {
    return static_cast<uint16_t>(analogRead(pins::ACT_FEEDBACK));
}

}  // namespace actuator
