// Brush-arm servo (DFRobot SER0006) and bucket-arm servo (Parallax Standard Servo, D028), driven
// by the Arduino Servo library on Timer1 (D020).
#pragma once

#include <Arduino.h>

enum class ServoId : uint8_t { BRUSH = 0, BUCKET = 1 };

namespace servos {

// Holds both signal lines low without pulses, so the servos do not move at boot.
void begin();

// Sends pulses of `pulse_us`, clamped to the signal range of that servo (config.h). Attaches the
// servo on the first call. Returns the pulse width used, or 0 when the bucket is refused because
// safety::bucket_may_move() is false. While the bucket gets pulses, the tracks are locked.
uint16_t set_pulse_us(ServoId id, uint16_t pulse_us);

// Stops the pulses (detaches the servo). How the servo behaves without a signal is a bench
// check (D008).
void release(ServoId id);

// Current pulse width in us, or 0 when released.
uint16_t pulse_us(ServoId id);

}  // namespace servos
