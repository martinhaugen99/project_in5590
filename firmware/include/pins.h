// Pin map of M.A.T.S on the Arduino Leonardo (D020). The wiring sheets in electronics/wiring/
// are the source of truth (D015): 01_signals.yml (sensors, display, servos, encoders) and
// 02_drivers.yml (motor drivers). The sheets use the Leonardo pin labels (D0..D13, A0..A5,
// ICSP-1..ICSP-4).
//
// The static_asserts check each pin against the Leonardo variant of the Arduino AVR core
// (variants/leonardo/pins_arduino.h in framework-arduino-avr 5.4.0, which atmelavr 5.3.0
// installs), so a build for another board fails here instead of driving the wrong pins.
#pragma once

#include <Arduino.h>

namespace pins {

// Motor drivers (sheet 02). MOTOR_ENABLE drives 1,2EN and 3,4EN of U2 and 1,2EN of U3 through
// R1 (1 kOhm), with R2 (10 kOhm) to GND, so both chips are off while D12 floats (also during
// reset, ATmega32U4 datasheet 7766J section 10.2.1).
constexpr uint8_t MOTOR_ENABLE = 12;
// Track motors on U2. Only IN1 of each motor has PWM: D6 is OC4D (Timer4), D11 is OC0A
// (Timer0). D7 and D9 are plain digital outputs; D9 has no PWM because the Servo library takes
// Timer1 (D020).
constexpr uint8_t LEFT_IN1 = 6;    // U2 pin 2 (1A)
constexpr uint8_t LEFT_IN2 = 7;    // U2 pin 7 (2A)
constexpr uint8_t RIGHT_IN1 = 11;  // U2 pin 10 (3A)
constexpr uint8_t RIGHT_IN2 = 9;   // U2 pin 15 (4A)
// PQ12 linear actuator on U3, digital outputs only (D10 is on Timer1, taken by the Servo
// library).
constexpr uint8_t ACT_IN1 = 8;   // U3 pin 2 (1A)
constexpr uint8_t ACT_IN2 = 10;  // U3 pin 7 (2A)

// Encoders (sheet 01, J12/J13): phase A only, on external interrupts INT2 (D0) and INT3 (D1).
// D0/D1 are also Serial1, which the firmware does not use.
constexpr uint8_t ENC_LEFT_A = 0;
constexpr uint8_t ENC_RIGHT_A = 1;

// Edge sensors (J7/J8): two VL53L4CD on the hardware I2C bus (D2 SDA, D3 SCL), each with its
// own XSHUT pin, driven open-drain (D020). D5 goes to the left sensor (J7) and D4 to the right
// sensor (J8), as wired on the robot (found on the bench 2026-11-22, D020 pin table).
constexpr uint8_t TOF_LEFT_XSHUT = 5;
constexpr uint8_t TOF_RIGHT_XSHUT = 13;
constexpr uint8_t I2C_SDA = 2;
constexpr uint8_t I2C_SCL = 3;

// Display (J9): ILI9341 on the hardware SPI of the ICSP header (ICSP-3 SCK, ICSP-4 COPI),
// through resistor dividers. RESET is tied to 3V3 through R3 (10 kOhm). CIPO (ICSP-1) is not
// connected.
constexpr uint8_t TFT_DC = A2;
constexpr uint8_t TFT_CS = A3;
constexpr uint8_t TFT_MOSI = MOSI;
constexpr uint8_t TFT_SCK = SCK;

// Analog inputs, referenced to AVCC (the Leonardo 5 V).
constexpr uint8_t ACT_FEEDBACK = A0;  // PQ12 pot wiper, between GND and +5V_USB (D020)
constexpr uint8_t BUTTON_ADC = A1;    // button ladder, not documented yet (sheet 01)

// Servo signals (J10/J11), powered from +5V_SERVO.
constexpr uint8_t BRUSH_SERVO = A4;
constexpr uint8_t BUCKET_SERVO = A5;

}  // namespace pins

static_assert(pins::MOTOR_ENABLE == 12, "MOTOR_ENABLE must be D12");
static_assert(pins::I2C_SDA == SDA && pins::I2C_SCL == SCL, "I2C must be on D2 (SDA), D3 (SCL)");
static_assert(pins::TFT_MOSI == 16 && pins::TFT_SCK == 15, "SPI must be on the ICSP header");
static_assert(pins::ACT_FEEDBACK == 18, "A0 must be pin 18");
static_assert(pins::BUTTON_ADC == 19, "A1 must be pin 19");
static_assert(pins::TFT_DC == 20, "A2 must be pin 20");
static_assert(pins::TFT_CS == 21, "A3 must be pin 21");
static_assert(pins::BRUSH_SERVO == 22, "A4 must be pin 22");
static_assert(pins::BUCKET_SERVO == 23, "A5 must be pin 23");
