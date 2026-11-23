// Track motors (L293D U2) and wheel encoders (phase A only).
#pragma once

#include <Arduino.h>

enum class Side : uint8_t { LEFT = 0, RIGHT = 1 };

namespace tracks {

// Sets every driver input low, keeps both L293D chips disabled, sets Timer4 to the same PWM
// frequency as Timer0 and starts the encoder interrupts.
void begin();

// Enables or disables both L293D chips. MOTOR_ENABLE is shared by the track driver (U2) and
// the actuator driver (U3), so this also enables or disables the actuator.
void set_driver_enable(bool on);
bool driver_enabled();

// Runs one track at `duty_percent` (positive is forward) and stops it after `run_ms`. The duty
// is capped at +-config::TRACK_MAX_DUTY_PCT and the run time at config::TRACK_MAX_RUN_MS.
// Forward is refused while safety::forward_allowed() is false, and reverse while
// safety::REVERSE_ALLOWED is false. A refused command stops the track and returns 0; check
// duty() to see whether it was taken. Returns the run time that is used. Integer percent keeps
// float math out of the firmware, which saves about 1.5 KB of flash.
uint32_t run(Side side, int8_t duty_percent, uint32_t run_ms);

// Sets both duties to zero. With the drivers enabled, this is a fast motor stop (L293D
// datasheet, Table 3).
void stop();

// Stops a track when its run time has passed, or when the edge gate or the interlock no
// longer allows its duty (safety.h). Call in every loop tick, after the edge sensors.
void update(uint32_t now_ms);

// Commanded duty in percent (-100 to 100).
int8_t duty(Side side);

// Encoder counts since the last reset. The encoders have no phase B, so the sign of each
// count comes from the direction of the last nonzero command.
int32_t counts(Side side);
void reset_counts();

}  // namespace tracks
