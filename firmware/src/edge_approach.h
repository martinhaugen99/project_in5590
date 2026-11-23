// Edge approach for the optimization task (D022, D023). High level: both tracks drive forward at
// one duty. Each track stops for the rest of the run when its sensor stops seeing the table, so
// the robot turns about the first track that stops and ends square to the edge. It only sends
// track commands, and the edge gate and the interlock in the low level still apply (safety.h).
#pragma once

#include <Arduino.h>

namespace edge_approach {

enum class State : uint8_t {
    IDLE,
    STARTING,  // the tracks are stopped, waiting for fresh readings after the display draw
    DRIVING,   // both tracks drive forward
    SQUARING,  // one track has stopped, the other drives on
    DONE,      // both tracks stopped at the edge
    ABORTED,   // a timeout, a sensor fault, the interlock or `stop` ended the run
};

// Starts a run at `duty_percent`. Refused, with the reason on the console, when a run is active,
// the duty is outside 1..config::TRACK_MAX_DUTY_PCT, the drivers are off, a track is moving
// (D024), or a track may not drive forward. Draws "Working" before the tracks start, which
// blocks for at least 154 ms (display.h). Resets the encoder counts.
bool start(int8_t duty_percent);

// Stops both tracks and ends an active run as stopped. Does nothing when no run is active.
void abort();

// Runs the approach. Call in every loop tick, after edge_sensors::update() and before
// tracks::update(). At the end of a run it prints one CSV result line and draws the state.
void update(uint32_t now_ms);

bool active();
State state();

}  // namespace edge_approach
