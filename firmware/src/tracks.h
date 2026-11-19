// Track motors (L293D U2) and wheel encoders (phase A only).
#pragma once

#include <Arduino.h>

enum class Side : uint8_t { LEFT = 0, RIGHT = 1 };

namespace tracks {

// Sets every driver input low, keeps both L293D chips disabled and starts the encoder
// interrupts. Returns false if a PWM channel could not be attached.
bool begin();

// Enables or disables both L293D chips. MOTOR_ENABLE is shared by the track driver (U2) and
// the actuator driver (U3), so this also enables or disables the actuator.
void set_driver_enable(bool on);
bool driver_enabled();

// Runs one track at `duty` (-1.0 to 1.0, positive is forward) and stops it after `run_ms`.
// The run time is capped at config::TRACK_MAX_RUN_MS. Returns the run time that is used.
uint32_t run(Side side, float duty, uint32_t run_ms);

// Sets both duties to zero. With the drivers enabled, this is a fast motor stop (L293D
// datasheet, Table 3).
void stop();

// Stops a track when its run time has passed. Call in every loop iteration.
void update(uint32_t now_ms);

float duty(Side side);

// Encoder counts since the last reset. The encoders have no phase B, so the sign of each
// count comes from the direction of the last nonzero command.
int32_t counts(Side side);
void reset_counts();

}  // namespace tracks
