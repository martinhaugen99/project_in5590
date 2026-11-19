// The two push buttons ("start/stop" and "bucket emptied") share one resistor ladder on A1.
// Step 1 only reports the raw level. The thresholds for each button are set in step 2 from
// the levels measured with `btn`.
#pragma once

#include <Arduino.h>

namespace buttons {

void begin();

// Raw 12-bit ADC value (0-4095).
uint16_t raw();

uint32_t millivolts();

}  // namespace buttons
