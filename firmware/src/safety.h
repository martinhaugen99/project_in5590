// Safety checks of the low level (D023). The track, bucket-servo and actuator modules ask these
// checks before they apply a command, so no caller can get around them, the console included.
#pragma once

#include <Arduino.h>

#include "tracks.h"

namespace safety {

// True when the sensor is running and its last reading is at most config::EDGE_STALE_MS old.
bool sensor_live(Side side);

// True when the sensor is live, its range status is 0 and the distance is at most
// config::EDGE_TABLE_MAX_MM. Every other case counts as "no table".
bool sees_table(Side side);

// True when the track may drive forward: its sensor sees the table, the bucket servo gets no
// pulses and the actuator is still.
bool forward_allowed(Side side);

// The robot may reverse only along the path it has just driven (CLAUDE.md). That needs the
// back-away step, which is not written yet, so step 2 refuses every reverse command.
constexpr bool REVERSE_ALLOWED = false;

// True when the bucket servo may get pulses: both tracks are stopped and neither sensor sees the
// table (CLAUDE.md, dump interlock).
bool bucket_may_move();

// True when the actuator may move: both tracks are stopped.
bool actuator_may_move();

// Motor watchdog (D024). A Timer0 compare-B interrupt runs about once per millisecond. When no
// kick has come for config::WATCHDOG_MS while a track or the actuator is commanded, the interrupt
// pulls MOTOR_ENABLE low, so both L293D chips turn off and the motors coast to a stop, even when
// the main loop hangs. Call begin_watchdog() once in setup(), after tracks::begin().
void begin_watchdog();

// Resets the watchdog, and arms it only while a track or the actuator is commanded. The control
// tick and every track and actuator command call it.
void kick_watchdog();

// True once after the watchdog has turned the drivers off. The caller then stops the modules and
// disables the drivers, so their state matches the pin.
bool take_watchdog_trip();

}  // namespace safety
