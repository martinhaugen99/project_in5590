// Named constants for the firmware. Each value states its source: a datasheet, the installed
// core or library, a bench check, or a limit chosen for safe bring-up. Values that need a
// measurement on the robot (edge thresholds, actuator end positions, servo angles, button
// levels) are added in step 2, after the bench checks in README.md.
#pragma once

#include <Arduino.h>

namespace config {

// --- Serial console --------------------------------------------------------------------------
// The Nano ESP32 console is USB CDC, which ignores the baud rate. The value matches
// monitor_speed in platformio.ini.
constexpr uint32_t CONSOLE_BAUD = 115200;
constexpr size_t CONSOLE_LINE_MAX = 96;
// Period of the CSV status lines for `stream on`. Chosen: 10 lines per second.
constexpr uint32_t STREAM_PERIOD_MS = 100;

// --- Bring-up safety limits (chosen, not measured) ------------------------------------------
// Longest run of one `drive` command. Step 1 has no edge reflex, so the tracks must be off
// the ground.
constexpr uint32_t TRACK_MAX_RUN_MS = 3000;
// Longest run of one `act` command: about 5 mm at the no-load speed of 10 mm/s of the
// PQ12 100:1 (PQ12 datasheet). The PQ12-P has no limit switches, so jog in short steps.
constexpr uint32_t ACT_MAX_JOG_MS = 500;

// --- PWM (LEDC). The ESP32-S3 has 8 channels, 4 timers and 14-bit timers (soc_caps.h). -------
// Track motors and actuator: PWM on the L293D inputs with the enable pin high. The L293D
// datasheet (SLRS008D) states no maximum input frequency. Its switching test uses a 5 kHz
// pulse rate (Figure 2), and the propagation and transition times (up to 800 ns and 300 ns,
// section 6.6) are below 1 % of a 200 us period.
constexpr uint32_t MOTOR_PWM_HZ = 5000;
constexpr uint8_t MOTOR_PWM_BITS = 10;
constexpr uint32_t MOTOR_PWM_MAX = (1u << MOTOR_PWM_BITS) - 1;

// Servos (DFRobot SER0006): signal range 500-2500 us, neutral 1500 us (DFRobot product page).
// The page states no frame period. 50 Hz (20 ms) is the standard RC servo frame.
constexpr uint32_t SERVO_PWM_HZ = 50;
constexpr uint8_t SERVO_PWM_BITS = 14;
constexpr uint32_t SERVO_PERIOD_US = 1000000u / SERVO_PWM_HZ;
constexpr uint32_t SERVO_MIN_US = 500;
constexpr uint32_t SERVO_MAX_US = 2500;

// --- Motor and actuator direction (not yet checked on the robot) -----------------------------
// Set during the bench checks in README.md: `drive L 30 500` must move the left track
// forward, and `act extend 300` must extend the actuator. Flip a flag if it moves the wrong way.
constexpr bool LEFT_MOTOR_REVERSED = false;
constexpr bool RIGHT_MOTOR_REVERSED = false;
constexpr bool ACT_EXTEND_ON_IN1 = true;

// --- Edge sensors (STM32duino VL53L4CD 1.0.5) -------------------------------------------------
// The library uses 8-bit I2C addresses, with 0x52 as the default (vl53l4cd_class.h). Each
// sensor gets its own address at boot through its XSHUT pin.
constexpr uint8_t TOF_LEFT_ADDRESS = 0x54;
constexpr uint8_t TOF_RIGHT_ADDRESS = 0x56;
// Library range 10-200 ms, library default 50 ms. Tuned in step 2 for the edge reflex.
constexpr uint32_t TOF_TIMING_BUDGET_MS = 50;
// How often the main loop asks each sensor for new data. Chosen: 10 polls per timing budget.
constexpr uint32_t TOF_POLL_PERIOD_MS = 5;

// --- IMU link (OpenLog Artemis) ----------------------------------------------------------------
// The baud rate is set in the OpenLog Artemis serial logging menu, so it is given with
// `imu on <baud>` instead of being fixed here.
constexpr size_t IMU_LINE_MAX = 256;

}  // namespace config
