#include "edge_sensors.h"

#include <Wire.h>
#include <vl53l4cd_class.h>

#include "config.h"
#include "pins.h"

namespace {

// XSHUT = -1: the library does not touch the XSHUT pins. It would drive them push-pull to
// 5 V (vl53l4cd_class.h, begin() and VL53L4CD_On()), above the 3.6 V limit of the sensor
// (DS13812 Rev 8, Table 11). This file drives them open-drain instead (D020).
VL53L4CD sensors_[2] = {
    VL53L4CD(&Wire, -1),
    VL53L4CD(&Wire, -1),
};
constexpr uint8_t XSHUT_PINS[2] = {pins::TOF_LEFT_XSHUT, pins::TOF_RIGHT_XSHUT};
constexpr uint8_t ADDRESSES[2] = {config::TOF_LEFT_ADDRESS, config::TOF_RIGHT_ADDRESS};

bool ok_[2] = {false, false};
uint8_t data_ready_level_[2] = {1, 1};  // GPIO_TIO_HV_STATUS bit 0 when a result is ready
EdgeReading readings_[2] = {};
uint32_t last_poll_ms_ = 0;

// Raw range status (bits 4:0 of RESULT_RANGE_STATUS) to the status the library reports, copied
// from status_rtn in VL53L4CD_GetResult() (vl53l4cd_api.cpp). 0 means a valid distance.
const uint8_t STATUS_MAP[24] PROGMEM = {255, 255, 255, 5,   2,   4,   1,  7, 3,   0,   255, 255,
                                        9,   13,  255, 255, 255, 255, 10, 6, 255, 255, 11,  12};

size_t index_of(Side side) {
    return static_cast<size_t>(side);
}

// Shutdown: output low.
void xshut_low(uint8_t pin) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
}

// Release: input with the port bit low, so the internal pull-up stays off and the 10 kOhm
// pull-up on the sensor board raises XSHUT to 3.3 V. pinMode(INPUT) clears the port bit
// (wiring_digital.c), which keeps the internal pull-up off.
void xshut_release(uint8_t pin) {
    pinMode(pin, INPUT);
}

// The library retries a failed register read forever (VL53L4CD_I2CRead in platform.cpp). While
// the motors ran, that hung the loop with the motors on (D024). The polling below uses these
// reads and writes instead: one try per transfer, bounded by the Wire timeout, and false on any
// failure. The library is still used to start the sensors at boot, while the motors are off.
uint8_t bus_address(size_t i) {
    return static_cast<uint8_t>(ADDRESSES[i] >> 1);  // the library uses 8-bit addresses
}

void send_register(uint8_t address, uint16_t reg) {
    Wire.beginTransmission(address);
    Wire.write(static_cast<uint8_t>(reg >> 8));
    Wire.write(static_cast<uint8_t>(reg & 0xFF));
}

bool read_register(uint8_t address, uint16_t reg, uint8_t* out, uint8_t size) {
    send_register(address, reg);
    // Repeated start before the read, as in VL53L4CD_I2CRead().
    if (Wire.endTransmission(false) != 0 || Wire.requestFrom(address, size) != size) {
        return false;
    }
    for (uint8_t k = 0; k < size; ++k) {
        out[k] = static_cast<uint8_t>(Wire.read());
    }
    return true;
}

bool write_register(uint8_t address, uint16_t reg, uint8_t value) {
    send_register(address, reg);
    Wire.write(value);
    return Wire.endTransmission(true) == 0;
}

// The same steps as VL53L4CD_CheckForDataReady(), VL53L4CD_ClearInterrupt() and
// VL53L4CD_GetResult() in vl53l4cd_api.cpp, reading only the range status, the SPAD count and
// the distance. Returns false when a transfer fails.
bool poll(size_t i, uint32_t now_ms) {
    const uint8_t address = bus_address(i);
    uint8_t value = 0;
    if (!read_register(address, VL53L4CD_GPIO_TIO_HV_STATUS, &value, 1)) {
        return false;
    }
    if ((value & 0x01) != data_ready_level_[i]) {
        return true;  // no new result yet
    }
    // Same order as the library example: clear the interrupt, then read the result.
    uint8_t status = 0;
    uint8_t spad[2] = {};
    uint8_t distance[2] = {};
    if (!write_register(address, VL53L4CD_SYSTEM_INTERRUPT_CLEAR, 0x01) ||
        !read_register(address, VL53L4CD_RESULT_RANGE_STATUS, &status, 1) ||
        !read_register(address, VL53L4CD_RESULT_SPAD_NB, spad, 2) ||
        !read_register(address, VL53L4CD_RESULT_DISTANCE, distance, 2)) {
        return false;
    }
    status &= 0x1F;
    if (status < sizeof(STATUS_MAP)) {
        status = pgm_read_byte(&STATUS_MAP[status]);
    }
    if (spad[0] == 0) {  // no SPADs in use: GetResult() reports status 255
        status = 255;
    }
    const uint16_t mm = static_cast<uint16_t>((distance[0] << 8) | distance[1]);
    readings_[i] = {true, status, mm, now_ms};
    return true;
}

bool start_sensor(size_t i) {
    xshut_release(XSHUT_PINS[i]);
    delay(config::TOF_BOOT_MS);
    // InitSensor() moves the sensor to its new address and checks its ID. The other sensor
    // stays in shutdown until its own turn.
    if (sensors_[i].InitSensor(ADDRESSES[i]) != VL53L4CD_ERROR_NONE) {
        return false;
    }
    if (sensors_[i].VL53L4CD_SetRangeTiming(config::TOF_TIMING_BUDGET_MS, 0) !=
        VL53L4CD_ERROR_NONE) {
        return false;
    }
    if (sensors_[i].VL53L4CD_StartRanging() != VL53L4CD_ERROR_NONE) {
        return false;
    }
    // Interrupt polarity, as in VL53L4CD_CheckForDataReady(): bit 4 of GPIO_HV_MUX_CTRL set
    // means that bit 0 of GPIO_TIO_HV_STATUS is 0 when a result is ready. It does not change
    // after the start, so it is read once here.
    uint8_t mux = 0;
    if (!read_register(bus_address(i), VL53L4CD_GPIO_HV_MUX_CTRL, &mux, 1)) {
        return false;
    }
    data_ready_level_[i] = (mux & 0x10) ? 0 : 1;
    return true;
}

}  // namespace

namespace edge_sensors {

void hold_in_shutdown() {
    for (const uint8_t pin : XSHUT_PINS) {
        xshut_low(pin);
    }
}

bool begin() {
    hold_in_shutdown();
    Wire.begin();
    // twi_init() turns on the internal pull-ups to 5 V on SDA and SCL (twi.c). Turn them off,
    // so only the 2.2 kOhm pull-ups to 3.3 V on the sensor boards remain (D020).
    digitalWrite(pins::I2C_SDA, LOW);
    digitalWrite(pins::I2C_SCL, LOW);
    Wire.setClock(config::I2C_CLOCK_HZ);
    // No reset on timeout: the reset calls twi_init(), which turns the 5 V pull-ups on again.
    Wire.setWireTimeout(config::I2C_TIMEOUT_US, false);

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
        if (ok_[i] && !poll(i, now_ms)) {
            ok_[i] = false;  // stays failed until the next reset
        }
    }
}

EdgeReading reading(Side side) {
    return readings_[index_of(side)];
}

}  // namespace edge_sensors
