#include "safety.h"

#include "actuator.h"
#include "config.h"
#include "edge_sensors.h"
#include "pins.h"
#include "servos.h"

namespace {

bool tracks_stopped() {
    return tracks::duty(Side::LEFT) == 0 && tracks::duty(Side::RIGHT) == 0;
}

// Shared with the interrupt. Each is one byte, so a read or a write cannot be split on the AVR.
volatile uint8_t ms_since_kick_ = 0;
volatile bool armed_ = false;
volatile bool tripped_ = false;
// Output register and bit of MOTOR_ENABLE, looked up once in begin_watchdog().
volatile uint8_t* enable_port_ = nullptr;
uint8_t enable_mask_ = 0;

}  // namespace

// Timer0 compare-B match: once per Timer0 cycle, 16 MHz / 64 / 256 = 1.024 ms (wiring.c). The
// interrupt runs with interrupts off, so this write cannot interleave with digitalWrite(), which
// also turns interrupts off while it changes the port (wiring_digital.c).
ISR(TIMER0_COMPB_vect) {
    if (!armed_) {
        return;
    }
    if (++ms_since_kick_ >= config::WATCHDOG_MS) {
        *enable_port_ &= static_cast<uint8_t>(~enable_mask_);
        armed_ = false;
        tripped_ = true;
    }
}

namespace safety {

bool sensor_live(Side side) {
    if (!edge_sensors::ok(side)) {
        return false;
    }
    const EdgeReading r = edge_sensors::reading(side);
    return r.valid && millis() - r.time_ms <= config::EDGE_STALE_MS;
}

bool sees_table(Side side) {
    if (!sensor_live(side)) {
        return false;
    }
    const EdgeReading r = edge_sensors::reading(side);
    return r.range_status == 0 && r.distance_mm <= config::EDGE_TABLE_MAX_MM;
}

bool forward_allowed(Side side) {
    return sees_table(side) && servos::pulse_us(ServoId::BUCKET) == 0 &&
           actuator::motion() == actuator::Motion::STOP;
}

bool bucket_may_move() {
    return tracks_stopped() && sensor_live(Side::LEFT) && sensor_live(Side::RIGHT) &&
           !sees_table(Side::LEFT) && !sees_table(Side::RIGHT);
}

bool actuator_may_move() {
    return tracks_stopped();
}

void begin_watchdog() {
    enable_port_ = portOutputRegister(digitalPinToPort(pins::MOTOR_ENABLE));
    enable_mask_ = digitalPinToBitMask(pins::MOTOR_ENABLE);
    // Timer0 already runs for millis(), and the core uses only its overflow interrupt
    // (wiring.c). OC0B stays disconnected (COM0B1:0 = 0), so D3 stays I2C SCL.
    OCR0B = 128;
    TIMSK0 |= _BV(OCIE0B);
}

void kick_watchdog() {
    const bool moving = !tracks_stopped() || actuator::motion() != actuator::Motion::STOP;
    ms_since_kick_ = 0;
    armed_ = moving;
}

bool take_watchdog_trip() {
    if (!tripped_) {
        return false;
    }
    tripped_ = false;
    return true;
}

}  // namespace safety
