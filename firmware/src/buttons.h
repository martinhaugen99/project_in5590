// The two push buttons ("start/stop" and "bucket emptied") share one resistor ladder on A1.
// The ladder is not on the wiring sheets yet (sheet 01), so the firmware only reports the raw
// level.
// The thresholds for each button are set later from the levels measured with `btn`.
#pragma once

#include <Arduino.h>

namespace buttons {

void begin();

// Raw 10-bit ADC value (0-1023), referenced to AVCC.
uint16_t raw();

}  // namespace buttons
