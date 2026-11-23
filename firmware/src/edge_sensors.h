// Two downward VL53L4CD time-of-flight edge sensors (D007), one ahead of each track, on one
// I2C bus. The edge gate in safety.h decides from their readings whether a track sees the table
// (D023). The bus has little noise margin (D026), so a failed poll is tried again, a sensor that
// keeps failing is marked faulted, and recover() restarts faulted sensors while nothing moves.
#pragma once

#include <Arduino.h>

#include "tracks.h"

struct EdgeReading {
    bool valid;            // at least one result has arrived
    uint8_t range_status;  // 0 means a valid distance (VL53L4CD_GetResult(), vl53l4cd_api.cpp)
    uint16_t distance_mm;
    uint32_t time_ms;  // millis() when the result was read
};

// Error codes of a failed transfer or start-up step. 1 to 5 are the results of
// Wire.endTransmission() (twi_writeTo() in twi.c of the Arduino AVR core). The rest are ours.
namespace tof_error {
constexpr uint8_t ADDRESS_NACK = 2;  // no device answered at the address
constexpr uint8_t DATA_NACK = 3;     // the device did not acknowledge a data byte
constexpr uint8_t BUS_ERROR = 4;     // lost arbitration or another bus error
constexpr uint8_t TIMEOUT = 5;       // the transfer did not finish within the Wire timeout
constexpr uint8_t SHORT_READ = 6;    // a read returned fewer bytes than asked, without a timeout
constexpr uint8_t WRONG_ID = 7;      // start-up: the model ID is not 0xEBAA
constexpr uint8_t NOT_READY = 8;     // start-up: a wait ran past config::TOF_START_WAIT_MS
}  // namespace tof_error

// Error counts and the last fault of one sensor, for the console.
struct EdgeHealth {
    uint16_t errors;       // failed polls and start-ups since the reset
    uint16_t recoveries;   // times the sensor ran again after a fault
    uint8_t last_code;     // tof_error code of the last failure, 0 if none yet
    bool own_address;      // at the last fault: the sensor answered at its own address
    bool default_address;  // at the last fault: a device answered at the default address 0x29
};

namespace edge_sensors {

// Holds both sensors in shutdown (XSHUT low). Call first in setup(), before anything else.
void hold_in_shutdown();

// Starts I2C with the internal pull-ups off, gives each sensor its own address through its
// XSHUT pin and starts continuous ranging. Returns true only if both sensors started. A sensor
// that did not start counts as faulted, and recover() tries it again.
bool begin();

// True while the sensor runs: it has started and has not faulted since.
bool ok(Side side);

// Polls for new results without waiting. Call in every control tick. A failed poll keeps the
// last reading. After config::TOF_FAULT_AFTER failed polls in a row, the sensor is faulted, and
// the firmware checks whether it still answers at its own address or at the default address.
void update(uint32_t now_ms);

// Brings faulted sensors back: a sensor that still answers at its own address runs on, and the
// others restart through XSHUT. A restart blocks the loop for the start-up (D026), so call this
// only while nothing moves. Tries at most once per config::TOF_RETRY_MS.
void recover(uint32_t now_ms);

EdgeReading reading(Side side);

EdgeHealth health(Side side);

}  // namespace edge_sensors
