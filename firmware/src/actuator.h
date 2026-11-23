// Actuonix PQ12-100-12-P linear actuator (brush push) on L293D U3, with the potentiometer
// wiper on A0. The -P model has no limit switches (PQ12 datasheet), so it only jogs in short,
// timed steps. Soft end limits come later, from the measured end positions.
#pragma once

#include <Arduino.h>

namespace actuator {

enum class Motion : int8_t { RETRACT = -1, STOP = 0, EXTEND = 1 };

// Sets both driver inputs low. The driver enable is shared with the tracks
// (tracks::set_driver_enable).
void begin();

// Moves at full speed for `run_ms`, capped at config::ACT_MAX_JOG_MS. Returns the run time
// used, or 0 when refused because a track is running (safety::actuator_may_move()). While the
// actuator moves, the tracks are locked.
uint32_t jog(Motion motion, uint32_t run_ms);

void stop();

// Stops the actuator when its jog time has passed. Call in every loop iteration.
void update(uint32_t now_ms);

Motion motion();

// Raw 10-bit ADC value of the potentiometer wiper (0-1023). The pot sits between GND and
// +5V_USB, and the ADC references AVCC, so the reading covers the full range (D020).
uint16_t feedback_raw();

}  // namespace actuator
