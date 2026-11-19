#include "actuator.h"

#include "config.h"
#include "pins.h"

namespace {

actuator::Motion motion_ = actuator::Motion::STOP;
uint32_t stop_at_ms_ = 0;

// The PQ12 extends with one polarity on motor pins 2 and 3 and retracts with the other
// (PQ12 datasheet). Which driver input gives "extend" depends on the wiring and is set in
// config::ACT_EXTEND_ON_IN1.
void write_motion(actuator::Motion motion) {
    const uint8_t extend_pin = config::ACT_EXTEND_ON_IN1 ? pins::ACT_IN1 : pins::ACT_IN2;
    const uint8_t retract_pin = config::ACT_EXTEND_ON_IN1 ? pins::ACT_IN2 : pins::ACT_IN1;
    switch (motion) {
        case actuator::Motion::EXTEND:
            ledcWrite(retract_pin, 0);
            ledcWrite(extend_pin, config::MOTOR_PWM_MAX);
            break;
        case actuator::Motion::RETRACT:
            ledcWrite(extend_pin, 0);
            ledcWrite(retract_pin, config::MOTOR_PWM_MAX);
            break;
        case actuator::Motion::STOP:
            ledcWrite(extend_pin, 0);
            ledcWrite(retract_pin, 0);
            break;
    }
}

}  // namespace

namespace actuator {

bool begin() {
    bool ok = ledcAttach(pins::ACT_IN1, config::MOTOR_PWM_HZ, config::MOTOR_PWM_BITS);
    ok = ledcAttach(pins::ACT_IN2, config::MOTOR_PWM_HZ, config::MOTOR_PWM_BITS) && ok;
    write_motion(Motion::STOP);
    motion_ = Motion::STOP;
    return ok;
}

uint32_t jog(Motion motion, uint32_t run_ms) {
    const uint32_t used_ms = min(run_ms, config::ACT_MAX_JOG_MS);
    motion_ = motion;
    stop_at_ms_ = millis() + used_ms;
    write_motion(motion);
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

uint32_t feedback_mv() {
    return analogReadMilliVolts(pins::ACT_FEEDBACK);
}

}  // namespace actuator
