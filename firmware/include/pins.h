// Pin map of M.A.T.S. The wiring sheets in electronics/wiring/ are the source of truth (D015):
// 01_signals.yml (sensors, display, servos, IMU, encoders) and 02_drivers.yml (motor drivers).
// The sheets use Arduino pin labels (D0..D13, A0..A7).
//
// platformio.ini defines BOARD_USES_HW_GPIO_NUMBERS, so the D*/A* constants of the core hold
// GPIO numbers and no pin remapping happens, also not inside libraries. The static_asserts
// check each pin against the official Nano ESP32 pinout (ABX00083 full pinout, page 3), so a
// build with the wrong numbering mode fails here instead of driving the wrong pins.
#pragma once

#include <Arduino.h>

namespace pins {

// Motor drivers (sheet 02). MOTOR_ENABLE drives 1,2EN and 3,4EN of both L293D chips through
// R1 (1 kOhm), with R2 (10 kOhm) to GND, so both chips are off while D0 floats.
// D0 is also UART0 RX of the ESP32-S3.
constexpr uint8_t MOTOR_ENABLE = D0;
constexpr uint8_t LEFT_IN1 = D6;   // U2 1A, left track motor
constexpr uint8_t LEFT_IN2 = D7;   // U2 2A, left track motor
constexpr uint8_t RIGHT_IN1 = D8;  // U2 3A, right track motor
constexpr uint8_t RIGHT_IN2 = D9;  // U2 4A, right track motor
constexpr uint8_t ACT_IN1 = D10;   // U3 1A, PQ12 linear actuator
// D1 is UART0 TX: the boot ROM prints on it during reset, while MOTOR_ENABLE is still low.
// The board has a 499 Ohm resistor in series with D1.
constexpr uint8_t ACT_IN2 = D1;  // U3 2A, PQ12 linear actuator

// Encoders (sheet 01, J12/J13): phase A only. Phase B and Z are not connected.
constexpr uint8_t ENC_LEFT_A = D2;
constexpr uint8_t ENC_RIGHT_A = D3;

// Edge sensors (J7/J8): two VL53L4CD on one I2C bus, with separate shutdown pins.
constexpr uint8_t TOF_LEFT_XSHUT = D4;
constexpr uint8_t TOF_RIGHT_XSHUT = D5;
constexpr uint8_t I2C_SDA = A4;
constexpr uint8_t I2C_SCL = A5;

// Display (J9): ILI9341 on SPI. RESET is tied to 3V3 through 10 kOhm, and SDO/MISO is not
// connected. D13 is also LED_BUILTIN, so the LED flickers with the SPI clock.
constexpr uint8_t TFT_MOSI = D11;
constexpr uint8_t TFT_SCK = D13;
constexpr uint8_t TFT_DC = A2;
constexpr uint8_t TFT_CS = A3;

// IMU: OpenLog Artemis TX into D12 (Serial1 RX). D12 is the default SPI MISO pin, so the
// display SPI must start without MISO.
constexpr uint8_t IMU_RX = D12;

// Analog inputs.
constexpr uint8_t ACT_FEEDBACK = A0;  // PQ12 potentiometer wiper, referenced to 3V3 and GND
constexpr uint8_t BUTTON_ADC = A1;    // both push buttons on one resistor ladder

// Servo signals (J10/J11), powered from +5V_SERVO.
constexpr uint8_t BRUSH_SERVO = A6;
constexpr uint8_t BUCKET_SERVO = A7;

}  // namespace pins

static_assert(pins::MOTOR_ENABLE == 44, "D0 must be GPIO44: check BOARD_USES_HW_GPIO_NUMBERS");
static_assert(pins::ACT_IN2 == 43, "D1 must be GPIO43");
static_assert(pins::ENC_LEFT_A == 5, "D2 must be GPIO5");
static_assert(pins::ENC_RIGHT_A == 6, "D3 must be GPIO6");
static_assert(pins::TOF_LEFT_XSHUT == 7, "D4 must be GPIO7");
static_assert(pins::TOF_RIGHT_XSHUT == 8, "D5 must be GPIO8");
static_assert(pins::LEFT_IN1 == 9, "D6 must be GPIO9");
static_assert(pins::LEFT_IN2 == 10, "D7 must be GPIO10");
static_assert(pins::RIGHT_IN1 == 17, "D8 must be GPIO17");
static_assert(pins::RIGHT_IN2 == 18, "D9 must be GPIO18");
static_assert(pins::ACT_IN1 == 21, "D10 must be GPIO21");
static_assert(pins::TFT_MOSI == 38, "D11 must be GPIO38");
static_assert(pins::IMU_RX == 47, "D12 must be GPIO47");
static_assert(pins::TFT_SCK == 48, "D13 must be GPIO48");
static_assert(pins::ACT_FEEDBACK == 1, "A0 must be GPIO1");
static_assert(pins::BUTTON_ADC == 2, "A1 must be GPIO2");
static_assert(pins::TFT_DC == 3, "A2 must be GPIO3");
static_assert(pins::TFT_CS == 4, "A3 must be GPIO4");
static_assert(pins::I2C_SDA == 11, "A4 must be GPIO11");
static_assert(pins::I2C_SCL == 12, "A5 must be GPIO12");
static_assert(pins::BRUSH_SERVO == 13, "A6 must be GPIO13");
static_assert(pins::BUCKET_SERVO == 14, "A7 must be GPIO14");
