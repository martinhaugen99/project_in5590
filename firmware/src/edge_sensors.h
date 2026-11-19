// Two downward VL53L4CD time-of-flight edge sensors (D007), one ahead of each track, on one
// I2C bus. Step 1 only reads them. The edge reflex comes in step 2.
#pragma once

#include <Arduino.h>

#include "tracks.h"

struct EdgeReading {
    bool valid;            // at least one result has arrived
    uint8_t range_status;  // 0 means a valid distance (STM32duino VL53L4CD, vl53l4cd_api.h)
    uint16_t distance_mm;
    uint32_t time_ms;  // millis() when the result was read
};

namespace edge_sensors {

// Starts I2C, gives each sensor its own address through its XSHUT pin and starts continuous
// ranging. Returns true only if both sensors started. Check ok() for each sensor.
bool begin();

bool ok(Side side);

// Polls for new results without waiting. Call in every loop iteration.
void update(uint32_t now_ms);

EdgeReading reading(Side side);

}  // namespace edge_sensors
