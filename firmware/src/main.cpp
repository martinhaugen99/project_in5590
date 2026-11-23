// M.A.T.S firmware for the Arduino Leonardo, step 2: edge gate and edge approach (D013, D020,
// D023). A serial console tests each part and starts the edge approach for the optimization
// task (D022). The rest of the dump cycle comes later.
#include <Arduino.h>

#include "actuator.h"
#include "buttons.h"
#include "config.h"
#include "console.h"
#include "display.h"
#include "edge_approach.h"
#include "edge_sensors.h"
#include "pins.h"
#include "safety.h"
#include "servos.h"
#include "tracks.h"

namespace {

// Longest wait for the laptop to open the USB console, so the boot messages are not lost.
// Chosen value; the robot also starts without a console.
constexpr uint32_t CONSOLE_WAIT_MS = 2000;

uint32_t last_tick_ms_ = 0;

void report(const __FlashStringHelper* part, bool ok) {
    Serial.print(part);
    Serial.println(ok ? F(": ok") : F(": FAILED"));
}

}  // namespace

void setup() {
    // First: hold both L293D chips off and both edge sensors in shutdown. R2 already pulls
    // MOTOR_ENABLE low while D12 floats, and the sensor boards pull XSHUT up to 3.3 V.
    digitalWrite(pins::MOTOR_ENABLE, LOW);
    pinMode(pins::MOTOR_ENABLE, OUTPUT);
    edge_sensors::hold_in_shutdown();

    Serial.begin(config::CONSOLE_BAUD);
    const uint32_t start_ms = millis();
    while (!Serial && millis() - start_ms < CONSOLE_WAIT_MS) {
        delay(10);
    }

    tracks::begin();
    safety::begin_watchdog();
    actuator::begin();
    servos::begin();
    buttons::begin();
    report(F("edge sensors"), edge_sensors::begin());
    if (!edge_sensors::ok(Side::LEFT) || !edge_sensors::ok(Side::RIGHT)) {
        Serial.print(F("  left "));
        Serial.print(edge_sensors::ok(Side::LEFT) ? F("ok") : F("FAILED"));
        Serial.print(F(", right "));
        Serial.println(edge_sensors::ok(Side::RIGHT) ? F("ok") : F("FAILED"));
    }
    display::begin();
    display::show_text("M.A.T.S");
    console::begin();
}

void loop() {
    const uint32_t now_ms = millis();
    // Control tick (D023). The order matters: fresh sensor data first, then the edge approach,
    // then the edge gate and the timers in the low level, so the edge check runs before a
    // track command takes effect.
    if (now_ms - last_tick_ms_ >= config::LOOP_PERIOD_MS) {
        last_tick_ms_ = now_ms;
        if (safety::take_watchdog_trip()) {
            // The interrupt has already pulled MOTOR_ENABLE low. Make the modules match.
            Serial.println(F("watchdog: loop stalled, drivers off (`en 1` to go on)"));
            edge_approach::abort();
            tracks::stop();
            actuator::stop();
            tracks::set_driver_enable(false);
        }
        edge_sensors::update(now_ms);
        edge_approach::update(now_ms);
        tracks::update(now_ms);
        actuator::update(now_ms);
        safety::kick_watchdog();
    }
    // Console commands also go through the safety checks (safety.h).
    console::update(now_ms);
}
