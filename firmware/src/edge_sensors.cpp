#include "edge_sensors.h"

#include <Wire.h>
#include <vl53l4cd_class.h>

#include "config.h"
#include "pins.h"

namespace {

VL53L4CD sensors_[2] = {
    VL53L4CD(&Wire, pins::TOF_LEFT_XSHUT),
    VL53L4CD(&Wire, pins::TOF_RIGHT_XSHUT),
};
constexpr uint8_t ADDRESSES[2] = {config::TOF_LEFT_ADDRESS, config::TOF_RIGHT_ADDRESS};

bool ok_[2] = {false, false};
EdgeReading readings_[2] = {};
uint32_t last_poll_ms_ = 0;

size_t index_of(Side side) {
    return static_cast<size_t>(side);
}

bool start_sensor(size_t i) {
    // InitSensor() powers the sensor up through its own XSHUT pin, moves it to the new
    // address and checks its ID. The other sensor stays in shutdown until its own turn.
    if (sensors_[i].InitSensor(ADDRESSES[i]) != VL53L4CD_ERROR_NONE) {
        return false;
    }
    if (sensors_[i].VL53L4CD_SetRangeTiming(config::TOF_TIMING_BUDGET_MS, 0) !=
        VL53L4CD_ERROR_NONE) {
        return false;
    }
    return sensors_[i].VL53L4CD_StartRanging() == VL53L4CD_ERROR_NONE;
}

}  // namespace

namespace edge_sensors {

bool begin() {
    Wire.begin(pins::I2C_SDA, pins::I2C_SCL);
    // Hold both sensors in shutdown, so each one can be moved to its own address alone.
    for (VL53L4CD& sensor : sensors_) {
        sensor.begin();
    }
    for (size_t i = 0; i < 2; ++i) {
        ok_[i] = start_sensor(i);
    }
    return ok_[0] && ok_[1];
}

bool ok(Side side) {
    return ok_[index_of(side)];
}

void update(uint32_t now_ms) {
    if (now_ms - last_poll_ms_ < config::TOF_POLL_PERIOD_MS) {
        return;
    }
    last_poll_ms_ = now_ms;

    for (size_t i = 0; i < 2; ++i) {
        if (!ok_[i]) {
            continue;
        }
        uint8_t ready = 0;
        if (sensors_[i].VL53L4CD_CheckForDataReady(&ready) != VL53L4CD_ERROR_NONE || !ready) {
            continue;
        }
        // Same order as the library example: clear the interrupt, then read the result.
        sensors_[i].VL53L4CD_ClearInterrupt();
        VL53L4CD_Result_t result = {};
        if (sensors_[i].VL53L4CD_GetResult(&result) == VL53L4CD_ERROR_NONE) {
            readings_[i] = {true, result.range_status, result.distance_mm, now_ms};
        }
    }
}

EdgeReading reading(Side side) {
    return readings_[index_of(side)];
}

}  // namespace edge_sensors
