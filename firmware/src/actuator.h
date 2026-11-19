// Actuonix PQ12-100-12-P linear actuator (brush push) on L293D U3, with the potentiometer
// wiper on A0. The -P model has no limit switches (PQ12 datasheet), so step 1 only jogs it in
// short, timed steps. Soft end limits come in step 2, from the measured end positions.
#pragma once

#include <Arduino.h>

namespace actuator {

enum class Motion : int8_t { RETRACT = -1, STOP = 0, EXTEND = 1 };

// Attaches both driver inputs at zero duty. Returns false if a PWM channel could not be
// attached. The driver enable is shared with the tracks (tracks::set_driver_enable).
bool begin();

// Moves at full speed for `run_ms`, capped at config::ACT_MAX_JOG_MS. Returns the run time used.
uint32_t jog(Motion motion, uint32_t run_ms);

void stop();

// Stops the actuator when its jog time has passed. Call in every loop iteration.
void update(uint32_t now_ms);

Motion motion();

// Potentiometer wiper voltage in mV (0 to 3300 mV between the GND and 3V3 references).
uint32_t feedback_mv();

}  // namespace actuator
