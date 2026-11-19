// Brush-arm and bucket-arm servos (DFRobot SER0006), driven by LEDC at 50 Hz. No servo
// library: the pulse width is written directly.
#pragma once

#include <Arduino.h>

enum class ServoId : uint8_t { BRUSH = 0, BUCKET = 1 };

namespace servos {

// Attaches the PWM channels without pulses, so the servos do not move at boot. Returns false
// if a PWM channel could not be attached.
bool begin();

// Sends pulses of `pulse_us`, clamped to the SER0006 signal range. Returns the pulse width used.
uint32_t set_pulse_us(ServoId id, uint32_t pulse_us);

// Stops the pulses. How the servo behaves without a signal is a bench check (D008).
void release(ServoId id);

// Current pulse width in us, or 0 when released.
uint32_t pulse_us(ServoId id);

}  // namespace servos
