// Named constants for the firmware. Each value states its source: a datasheet, the installed
// core or library, a measurement on the robot, or a chosen limit. The step 2 values are
// explained in D023 and are to be checked in docs/experiments/001-edge-calibration.md.
#pragma once

#include <Arduino.h>

namespace config {

// --- Serial console --------------------------------------------------------------------------
// The Leonardo console is USB CDC, which ignores the baud rate. The value matches
// monitor_speed in platformio.ini.
constexpr uint32_t CONSOLE_BAUD = 115200;
// Longest command line. Chosen: the longest command is about 30 characters.
constexpr uint8_t CONSOLE_LINE_MAX = 48;
// Period of the CSV status lines for `stream on`. Chosen: 10 lines per second.
constexpr uint32_t STREAM_PERIOD_MS = 100;

// --- Bring-up safety limits (chosen, not measured) ------------------------------------------
// Longest run of one track command, such as `drive`.
constexpr uint32_t TRACK_MAX_RUN_MS = 3000;
// Longest run of one `act` command: about 5 mm at the no-load speed of 10 mm/s of the
// PQ12 100:1 (PQ12 datasheet). The PQ12-P has no limit switches, so jog in short steps.
constexpr uint32_t ACT_MAX_JOG_MS = 500;

// --- Track PWM ---------------------------------------------------------------------------------
// analogWrite() takes 0..255 (Arduino AVR core, wiring_analog.c). Timer0 (D11) runs fast PWM
// with prescaler 64, 16 MHz / 64 / 256 = 977 Hz. tracks::begin() switches Timer4 (D6) from
// phase and frequency correct to fast PWM, so both tracks run at about 977 Hz (ATmega32U4
// datasheet 7766J, Table 15-18; OCR4C resets to 0xFF, section 15.12).
constexpr uint8_t MOTOR_PWM_MAX = 255;

// --- Servos ----------------------------------------------------------------------------------
// The Servo library sends a 20 ms frame (Servo.h, REFRESH_INTERVAL). Each servo is clamped to
// its own signal range (D028).
// Brush arm, DFRobot SER0006: 500-2500 us, neutral 1500 us (DFRobot product page).
constexpr uint16_t BRUSH_SERVO_MIN_US = 500;
constexpr uint16_t BRUSH_SERVO_MAX_US = 2500;
// Bucket arms, Parallax Standard Servo #900-00005: 0.75-2.25 ms high pulse every 20 ms, center
// 1.5 ms (Parallax datasheet v2.2).
constexpr uint16_t BUCKET_SERVO_MIN_US = 750;
constexpr uint16_t BUCKET_SERVO_MAX_US = 2250;

// --- Motor and actuator direction -------------------------------------------------------------
// Checked on the bench on 2026-11-22 (README.md, bench checklist): `drive L 30 500` and
// `drive R 30 500` move each track forward. Flip a flag if a part is rewired.
constexpr bool LEFT_MOTOR_REVERSED = false;
constexpr bool RIGHT_MOTOR_REVERSED = true;
constexpr bool ACT_EXTEND_ON_IN1 = true;

// --- Edge sensors (VL53L4CD) ----------------------------------------------------------------
// 7-bit I2C addresses. A sensor answers at 0x29 after it boots (DS13812 Rev 8, section 3, gives
// the 8-bit form 0x52). Each sensor gets its own address at start-up through its XSHUT pin.
constexpr uint8_t TOF_DEFAULT_ADDRESS = 0x29;
constexpr uint8_t TOF_LEFT_ADDRESS = 0x2A;
constexpr uint8_t TOF_RIGHT_ADDRESS = 0x2B;
// I2C standard mode (D026). The bus has little noise margin: the 3.3 V pull-ups are below the
// 0.7 VCC = 3.5 V that the ATmega32U4 needs for a high (7766J, Table 29-4). 100 kHz gives the
// slowest edges and the longest bit times on the long sensor wires. A poll of both sensors then
// takes about 1 ms, and up to about 5 ms when both have a new result.
constexpr uint32_t I2C_CLOCK_HZ = 100000;
// Longest wait for one I2C transfer. The longest transfer in edge_sensors.cpp is a 4-byte write:
// 7 bytes of 9 bits, about 0.65 ms at 100 kHz. Chosen: 5 ms, so a tick with a failed transfer
// on both sensors stays well below the motor watchdog (WATCHDOG_MS).
constexpr uint32_t I2C_TIMEOUT_US = 5000;
// A sensor counts as faulted after this many failed polls in a row (D026). One failed poll keeps
// the last reading and tries again in the next tick, and EDGE_STALE_MS still stops a track whose
// reading gets old. Chosen: 3 polls, 15 ms.
constexpr uint8_t TOF_FAULT_AFTER = 3;
// A faulted sensor is restarted at most this often, and only while nothing moves (D026).
// Chosen: a restart blocks the loop for the start-up of the sensor.
constexpr uint32_t TOF_RETRY_MS = 1000;
// The sensor boots in at most 1.2 ms after XSHUT rises (DS13812 Rev 8, section 2.7).
// Chosen: 2 ms.
constexpr uint32_t TOF_BOOT_MS = 2;
// Longest wait for each step of the start-up: the boot, the VHV calibration and the first
// result. 1000 polls of 1 ms each, the same limit as the wait loops in VL53L4CD_SensorInit() and
// VL53L4CD_StartRanging() (vl53l4cd_api.cpp, STM32duino VL53L4CD 1.0.5).
constexpr uint16_t TOF_START_WAIT_MS = 1000;
// VL53L4CD_SetRangeTiming() accepts 10-200 ms, and its default is 50 ms (vl53l4cd_api.cpp).
// Tuned in step 2 for the edge reflex.
constexpr uint32_t TOF_TIMING_BUDGET_MS = 50;
static_assert(TOF_TIMING_BUDGET_MS >= 10 && TOF_TIMING_BUDGET_MS <= 200,
              "timing budget outside 10-200 ms");
// How often the main loop asks each sensor for new data. Chosen: 10 polls per timing budget.
constexpr uint32_t TOF_POLL_PERIOD_MS = 5;

// --- Step 2: edge gate and edge approach (D023) ----------------------------------------------
// Period of the control tick in main.cpp: edge sensors, edge approach, track gate. Chosen to
// match TOF_POLL_PERIOD_MS.
constexpr uint32_t LOOP_PERIOD_MS = 5;
// A sensor sees the table when its distance is at most this value with range status 0.
// Measured on 2026-11-22 with the brush down and its servo off: about 50 mm on the table and
// more than 100 mm past the edge. The threshold is halfway between (chosen).
constexpr uint16_t EDGE_TABLE_MAX_MM = 75;
// A reading older than this counts as "no table". Chosen: three timing budgets.
constexpr uint32_t EDGE_STALE_MS = 3 * TOF_TIMING_BUDGET_MS;
// Speed cap for every track command. Chosen, so the edge approach can test 50, 60 and 70 %. The
// optimization (D025) tunes the duty up to 100 %: raise the cap only after experiment 001 shows
// a gap at the edge at the new duty.
constexpr int8_t TRACK_MAX_DUTY_PCT = 70;
// Default duty of `approach`: the slowest duty that moved the robot reliably on the table with
// the brush down (measured on 2026-11-22).
constexpr int8_t APPROACH_DEFAULT_DUTY_PCT = 50;
// The edge approach renews each track command with this run time in every tick, so a track
// stops by itself when the approach stops renewing it. Chosen: 10 ticks.
constexpr uint32_t TRACK_LEASE_MS = 50;
// Longest edge approach, from the start of the tracks. Chosen: the GIF lasts at most 10 s.
constexpr uint32_t APPROACH_MAX_MS = 8000;
// Longest square-up turn after the first track has stopped. Chosen: ends a stalled turn.
constexpr uint32_t SQUARE_MAX_MS = 3000;
// Motor watchdog (D024): Timer0 cycles (1.024 ms each) without a kick before the interrupt turns
// the drivers off. Chosen: 10 control ticks, about 51 ms.
constexpr uint8_t WATCHDOG_MS = 50;

}  // namespace config
