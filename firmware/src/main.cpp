// M.A.T.S firmware, step 1: bench bring-up (D013). A serial console tests each part alone.
// There is no edge reflex yet, so every motion command is for the bench only, with the
// tracks off the ground. The safety layer and the dump cycle come in step 2.
#include <Arduino.h>

#include "actuator.h"
#include "buttons.h"
#include "config.h"
#include "console.h"
#include "display.h"
#include "edge_sensors.h"
#include "imu_link.h"
#include "pins.h"
#include "servos.h"
#include "tracks.h"

namespace {

// Longest wait for the laptop to open the USB console, so the boot messages are not lost.
// Chosen value; the robot also starts without a console.
constexpr uint32_t CONSOLE_WAIT_MS = 2000;

void report(const char* part, bool ok) {
    Serial.printf("%-13s %s\n", part, ok ? "ok" : "FAILED");
}

}  // namespace

void setup() {
    // First: hold both L293D chips off. R2 already pulls MOTOR_ENABLE low while D0 floats.
    pinMode(pins::MOTOR_ENABLE, OUTPUT);
    digitalWrite(pins::MOTOR_ENABLE, LOW);

    Serial.begin(config::CONSOLE_BAUD);
    const uint32_t start_ms = millis();
    while (!Serial && millis() - start_ms < CONSOLE_WAIT_MS) {
        delay(10);
    }

    report("tracks", tracks::begin());
    report("actuator", actuator::begin());
    report("servos", servos::begin());
    buttons::begin();
    report("edge sensors", edge_sensors::begin());
    if (!edge_sensors::ok(Side::LEFT) || !edge_sensors::ok(Side::RIGHT)) {
        Serial.printf("  left %s, right %s\n", edge_sensors::ok(Side::LEFT) ? "ok" : "FAILED",
                      edge_sensors::ok(Side::RIGHT) ? "ok" : "FAILED");
    }
    display::begin();
    display::show_text("M.A.T.S");
    console::begin();
}

void loop() {
    const uint32_t now_ms = millis();
    tracks::update(now_ms);
    actuator::update(now_ms);
    edge_sensors::update(now_ms);
    imu_link::update(Serial, now_ms);
    console::update(now_ms);
}
