#include "edge_sensors.h"

#include <Wire.h>

#include "config.h"
#include "pins.h"

namespace {

// Registers of the VL53L4CD, from vl53l4cd_api.h and vl53l4cd_api.cpp in STM32duino VL53L4CD
// 1.0.5. The register index is 16 bits, high byte first (VL53L4CD_I2CRead() in platform.cpp).
constexpr uint16_t REG_I2C_ADDRESS = 0x0001;
constexpr uint16_t REG_OSC_FREQUENCY = 0x0006;  // read in VL53L4CD_SetRangeTiming()
constexpr uint16_t REG_VHV_CONFIG_TIMEOUT_MACROP_LOOP_BOUND = 0x0008;
constexpr uint16_t REG_0x000B = 0x000B;  // written in VL53L4CD_SensorInit(), no name given
constexpr uint16_t REG_0x0024 = 0x0024;  // written in VL53L4CD_SensorInit(), no name given
constexpr uint16_t REG_DEFAULT_CONFIG = 0x002D;
constexpr uint16_t REG_GPIO_HV_MUX_CTRL = 0x0030;
constexpr uint16_t REG_GPIO_TIO_HV_STATUS = 0x0031;
constexpr uint16_t REG_RANGE_CONFIG_A = 0x005E;
constexpr uint16_t REG_RANGE_CONFIG_B = 0x0061;
constexpr uint16_t REG_INTERMEASUREMENT_MS = 0x006C;
constexpr uint16_t REG_SYSTEM_INTERRUPT_CLEAR = 0x0086;
constexpr uint16_t REG_SYSTEM_START = 0x0087;
constexpr uint16_t REG_RESULT_RANGE_STATUS = 0x0089;
constexpr uint16_t REG_RESULT_SPAD_NB = 0x008C;
constexpr uint16_t REG_RESULT_DISTANCE = 0x0096;
constexpr uint16_t REG_FIRMWARE_SYSTEM_STATUS = 0x00E5;
constexpr uint16_t REG_MODEL_ID = 0x010F;
constexpr uint16_t MODEL_ID = 0xEBAA;
constexpr uint8_t BOOTED = 0x03;  // FIRMWARE_SYSTEM_STATUS when the boot has finished

// Values for registers 0x2D to 0x87, copied from VL53L4CD_DEFAULT_CONFIGURATION in
// vl53l4cd_api.cpp (STM32duino VL53L4CD 1.0.5), which carries this notice:
//
//   COPYRIGHT(c) 2021 STMicroelectronics
//
//   Redistribution and use in source and binary forms, with or without modification,
//   are permitted provided that the following conditions are met:
//     1. Redistributions of source code must retain the above copyright notice,
//        this list of conditions and the following disclaimer.
//     2. Redistributions in binary form must reproduce the above copyright notice,
//        this list of conditions and the following disclaimer in the documentation
//        and/or other materials provided with the distribution.
//     3. Neither the name of STMicroelectronics nor the names of its contributors
//        may be used to endorse or promote products derived from this software
//        without specific prior written permission.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// 0x30 = 0x11 sets an active-low interrupt, read back after the start (start_sensor()).
const uint8_t DEFAULT_CONFIG[] PROGMEM = {
    0x12, 0x00, 0x00, 0x11, 0x02, 0x00, 0x02, 0x08, 0x00, 0x08, 0x10, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0xff, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0b, 0x00, 0x00, 0x02, 0x14, 0x21,
    0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x38, 0xff, 0x01, 0x00, 0x08, 0x00,
    0x00, 0x01, 0xcc, 0x07, 0x01, 0xf1, 0x05, 0x00, 0xa0, 0x00, 0x80, 0x08, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x0f, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x05, 0x06, 0x06, 0x00,
    0x00, 0x02, 0xc7, 0xff, 0x9b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
static_assert(sizeof(DEFAULT_CONFIG) == REG_SYSTEM_START - REG_DEFAULT_CONFIG + 1,
              "the default configuration covers registers 0x2D to 0x87");

// Raw range status (bits 4:0 of RESULT_RANGE_STATUS) to the status the library reports, copied
// from status_rtn in VL53L4CD_GetResult() (vl53l4cd_api.cpp). 0 means a valid distance.
const uint8_t STATUS_MAP[24] PROGMEM = {255, 255, 255, 5,   2,   4,   1,  7, 3,   0,   255, 255,
                                        9,   13,  255, 255, 255, 255, 10, 6, 255, 255, 11,  12};

constexpr uint8_t XSHUT_PINS[2] = {pins::TOF_LEFT_XSHUT, pins::TOF_RIGHT_XSHUT};
constexpr uint8_t ADDRESSES[2] = {config::TOF_LEFT_ADDRESS, config::TOF_RIGHT_ADDRESS};

bool running_[2] = {false, false};
uint8_t fails_in_row_[2] = {0, 0};
uint8_t data_ready_level_[2] = {1, 1};  // GPIO_TIO_HV_STATUS bit 0 when a result is ready
EdgeReading readings_[2] = {};
EdgeHealth health_[2] = {};
uint32_t last_poll_ms_ = 0;
uint32_t last_try_ms_ = 0;

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

void start_bus() {
    Wire.begin();
    // twi_init() turns on the internal pull-ups to 5 V on SDA and SCL (twi.c). Turn them off,
    // so only the 2.2 kOhm pull-ups to 3.3 V on the sensor boards remain (D020).
    digitalWrite(pins::I2C_SDA, LOW);
    digitalWrite(pins::I2C_SCL, LOW);
    // twi_init() also sets the clock back to 100 kHz (TWI_FREQ), so set it after every start.
    Wire.setClock(config::I2C_CLOCK_HZ);
    // No reset on timeout: the reset calls twi_init(), which turns the 5 V pull-ups on again.
    // restart_bus() does the reset instead. Setting the timeout also clears the timeout flag.
    Wire.setWireTimeout(config::I2C_TIMEOUT_US, false);
}

// After a timeout, the TWI state machine stays busy with the abandoned transfer, so every later
// transfer would time out as well (twi_writeTo() and twi_readFrom() in twi.c wait for
// TWI_READY). A restart of the TWI clears it.
void restart_bus() {
    Wire.end();
    start_bus();
}

// Register reads and writes with one try each, bounded by the Wire timeout. The library retries
// a failed read forever (VL53L4CD_I2CRead() in platform.cpp), which hung the loop with the
// motors on (D024), so this file does not use it. Each returns 0 on success, or the error code
// of the first failed step (edge_sensors.h).
void send_register(uint8_t address, uint16_t reg) {
    Wire.beginTransmission(address);
    Wire.write(static_cast<uint8_t>(reg >> 8));
    Wire.write(static_cast<uint8_t>(reg & 0xFF));
}

uint8_t read_register(uint8_t address, uint16_t reg, uint8_t* out, uint8_t size) {
    send_register(address, reg);
    // Repeated start before the read, as in VL53L4CD_I2CRead().
    const uint8_t code = Wire.endTransmission(false);
    if (code != 0) {
        return code;
    }
    if (Wire.requestFrom(address, size) != size) {
        return Wire.getWireTimeoutFlag() ? tof_error::TIMEOUT : tof_error::SHORT_READ;
    }
    for (uint8_t k = 0; k < size; ++k) {
        out[k] = static_cast<uint8_t>(Wire.read());
    }
    return 0;
}

uint8_t write_registers(uint8_t address, uint16_t reg, const uint8_t* data, uint8_t size) {
    send_register(address, reg);
    Wire.write(data, size);
    return Wire.endTransmission(true);
}

uint8_t write_register(uint8_t address, uint16_t reg, uint8_t value) {
    return write_registers(address, reg, &value, 1);
}

// 16-bit registers are sent high byte first (VL53L4CD_WrWord() in platform.cpp).
uint8_t write_word(uint8_t address, uint16_t reg, uint16_t value) {
    const uint8_t data[2] = {static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value)};
    return write_registers(address, reg, data, 2);
}

// Reads `reg` once per millisecond until (value & mask) == expected, for at most
// config::TOF_START_WAIT_MS.
uint8_t wait_for(uint8_t address, uint16_t reg, uint8_t mask, uint8_t expected) {
    for (uint16_t ms = 0; ms < config::TOF_START_WAIT_MS; ++ms) {
        uint8_t value = 0;
        const uint8_t code = read_register(address, reg, &value, 1);
        if (code != 0) {
            return code;
        }
        if ((value & mask) == expected) {
            return 0;
        }
        delay(1);
    }
    return tof_error::NOT_READY;
}

// Writes RANGE_CONFIG_A or RANGE_CONFIG_B for the timing budget, as in
// VL53L4CD_SetRangeTiming(). `budget_us` is already shifted left by 12.
uint8_t write_range_config(uint8_t address, uint16_t reg, uint32_t budget_us,
                           uint32_t macro_period_us, uint8_t factor) {
    const uint32_t tmp = (macro_period_us * factor) >> 6;
    uint32_t ls_byte = ((budget_us + (tmp >> 1)) / tmp) - 1;
    uint16_t ms_byte = 0;
    while ((ls_byte & 0xFFFFFF00UL) > 0) {
        ls_byte >>= 1;
        ++ms_byte;
    }
    return write_word(address, reg, static_cast<uint16_t>((ms_byte << 8) + (ls_byte & 0xFF)));
}

// VL53L4CD_SetRangeTiming(budget, 0): continuous ranging, no inter-measurement period.
uint8_t set_range_timing(uint8_t address) {
    uint8_t osc[2] = {};
    uint8_t code = read_register(address, REG_OSC_FREQUENCY, osc, 2);
    if (code != 0) {
        return code;
    }
    const uint16_t osc_frequency = static_cast<uint16_t>((osc[0] << 8) | osc[1]);
    if (osc_frequency == 0) {
        return tof_error::NOT_READY;
    }
    const uint32_t macro_period_us = (2304UL * (0x40000000UL / osc_frequency)) >> 6;
    const uint8_t zero[4] = {};
    code = write_registers(address, REG_INTERMEASUREMENT_MS, zero, 4);
    if (code != 0) {
        return code;
    }
    const uint32_t budget_us = (config::TOF_TIMING_BUDGET_MS * 1000UL - 2500UL) << 12;
    code = write_range_config(address, REG_RANGE_CONFIG_A, budget_us, macro_period_us, 16);
    if (code != 0) {
        return code;
    }
    return write_range_config(address, REG_RANGE_CONFIG_B, budget_us, macro_period_us, 12);
}

// Waits for a result, as in VL53L4CD_CheckForDataReady(), then clears the interrupt.
uint8_t wait_and_clear(size_t i) {
    const uint8_t address = ADDRESSES[i];
    const uint8_t code = wait_for(address, REG_GPIO_TIO_HV_STATUS, 0x01, data_ready_level_[i]);
    if (code != 0) {
        return code;
    }
    return write_register(address, REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
}

// Starts sensor i. Its XSHUT pin must be low, and no other sensor may answer at the default
// address. The steps are those of InitSensor() (vl53l4cd_class.h), VL53L4CD_SensorInit(),
// VL53L4CD_SetRangeTiming() and VL53L4CD_StartRanging() (vl53l4cd_api.cpp), with every wait
// bounded. Returns 0 on success, or the error code of the first failed step.
uint8_t start_sensor(size_t i) {
    const uint8_t address = ADDRESSES[i];
    xshut_release(XSHUT_PINS[i]);
    delay(config::TOF_BOOT_MS);

    // InitSensor(): the new address first, then the model ID.
    uint8_t code = write_register(config::TOF_DEFAULT_ADDRESS, REG_I2C_ADDRESS, address);
    uint8_t id[2] = {};
    if (code == 0) {
        code = read_register(address, REG_MODEL_ID, id, 2);
    }
    if (code != 0) {
        return code;
    }
    if (static_cast<uint16_t>((id[0] << 8) | id[1]) != MODEL_ID) {
        return tof_error::WRONG_ID;
    }

    // VL53L4CD_SensorInit(): wait for the boot, load the default configuration.
    code = wait_for(address, REG_FIRMWARE_SYSTEM_STATUS, 0xFF, BOOTED);
    for (uint8_t k = 0; code == 0 && k < sizeof(DEFAULT_CONFIG); ++k) {
        code = write_register(address, REG_DEFAULT_CONFIG + k, pgm_read_byte(&DEFAULT_CONFIG[k]));
    }
    // The interrupt polarity, as in VL53L4CD_CheckForDataReady(): bit 4 of GPIO_HV_MUX_CTRL set
    // means that bit 0 of GPIO_TIO_HV_STATUS is 0 when a result is ready. It does not change
    // after this, so it is read once.
    uint8_t mux = 0;
    if (code == 0) {
        code = read_register(address, REG_GPIO_HV_MUX_CTRL, &mux, 1);
    }
    data_ready_level_[i] = (mux & 0x10) ? 0 : 1;
    // Start the VHV calibration, wait for it, then stop.
    if (code == 0) {
        code = write_register(address, REG_SYSTEM_START, 0x40);
    }
    if (code == 0) {
        code = wait_and_clear(i);
    }
    if (code == 0) {
        code = write_register(address, REG_SYSTEM_START, 0x00);
    }
    if (code == 0) {
        code = write_register(address, REG_VHV_CONFIG_TIMEOUT_MACROP_LOOP_BOUND, 0x09);
    }
    if (code == 0) {
        code = write_register(address, REG_0x000B, 0x00);
    }
    if (code == 0) {
        code = write_word(address, REG_0x0024, 0x0500);
    }
    if (code == 0) {
        code = set_range_timing(address);
    }
    // VL53L4CD_StartRanging(): 0x21 starts continuous ranging, since INTERMEASUREMENT_MS is 0.
    // The library then waits for the first result and clears it.
    if (code == 0) {
        code = write_register(address, REG_SYSTEM_START, 0x21);
    }
    if (code == 0) {
        code = wait_and_clear(i);
    }
    return code;
}

// The same steps as VL53L4CD_CheckForDataReady(), VL53L4CD_ClearInterrupt() and
// VL53L4CD_GetResult() in vl53l4cd_api.cpp, reading only the range status, the SPAD count and
// the distance. Returns 0 on success, or the error code of the first failed transfer.
uint8_t poll(size_t i, uint32_t now_ms) {
    const uint8_t address = ADDRESSES[i];
    uint8_t value = 0;
    uint8_t code = read_register(address, REG_GPIO_TIO_HV_STATUS, &value, 1);
    if (code != 0 || (value & 0x01) != data_ready_level_[i]) {
        return code;  // a failure, or no new result yet
    }
    // Same order as the library example: clear the interrupt, then read the result.
    uint8_t status = 0;
    uint8_t spad[2] = {};
    uint8_t distance[2] = {};
    code = write_register(address, REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    if (code == 0) {
        code = read_register(address, REG_RESULT_RANGE_STATUS, &status, 1);
    }
    if (code == 0) {
        code = read_register(address, REG_RESULT_SPAD_NB, spad, 2);
    }
    if (code == 0) {
        code = read_register(address, REG_RESULT_DISTANCE, distance, 2);
    }
    if (code != 0) {
        return code;
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
    return 0;
}

// True when a device acknowledges `address`: an empty write, as an I2C scanner does.
bool answers(uint8_t address) {
    Wire.beginTransmission(address);
    const uint8_t code = Wire.endTransmission(true);
    if (code == tof_error::TIMEOUT) {
        restart_bus();
    }
    return code == 0;
}

// Counts a failure of sensor i, and restarts the bus after a timeout, so the other sensor can
// still be polled.
void count_failure(size_t i, uint8_t code) {
    EdgeHealth& h = health_[i];
    h.last_code = code;
    if (h.errors < UINT16_MAX) {
        ++h.errors;
    }
    if (code == tof_error::TIMEOUT) {
        restart_bus();
    }
}

// Marks sensor i as faulted, and records where it still answers (D026). A sensor that answers
// only at the default address has reset itself, after a glitch on XSHUT or its supply. One that
// still answers at its own address had a bus error.
void fault(size_t i) {
    running_[i] = false;
    health_[i].own_address = answers(ADDRESSES[i]);
    health_[i].default_address = answers(config::TOF_DEFAULT_ADDRESS);
}

// Starts every sensor that is not running, one at a time, since each answers at the default
// address until it has its own. A sensor that fails to start is held in shutdown again, so it
// cannot answer at the default address while the next one starts.
void start_stopped_sensors() {
    for (size_t i = 0; i < 2; ++i) {
        if (!running_[i]) {
            xshut_low(XSHUT_PINS[i]);
        }
    }
    delay(config::TOF_BOOT_MS);  // XSHUT low time. Chosen: the same as the boot time
    for (size_t i = 0; i < 2; ++i) {
        if (running_[i]) {
            continue;
        }
        const uint8_t code = start_sensor(i);
        if (code == 0) {
            running_[i] = true;
            fails_in_row_[i] = 0;
            continue;
        }
        count_failure(i, code);
        fault(i);
        xshut_low(XSHUT_PINS[i]);
    }
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
    start_bus();
    start_stopped_sensors();
    return running_[0] && running_[1];
}

bool ok(Side side) {
    return running_[index_of(side)];
}

void update(uint32_t now_ms) {
    if (now_ms - last_poll_ms_ < config::TOF_POLL_PERIOD_MS) {
        return;
    }
    last_poll_ms_ = now_ms;

    for (size_t i = 0; i < 2; ++i) {
        if (!running_[i]) {
            continue;
        }
        const uint8_t code = poll(i, now_ms);
        if (code == 0) {
            fails_in_row_[i] = 0;
            continue;
        }
        count_failure(i, code);
        if (++fails_in_row_[i] >= config::TOF_FAULT_AFTER) {
            fault(i);
        }
    }
}

void recover(uint32_t now_ms) {
    if ((running_[0] && running_[1]) || now_ms - last_try_ms_ < config::TOF_RETRY_MS) {
        return;
    }
    last_try_ms_ = now_ms;
    const bool was_running[2] = {running_[0], running_[1]};
    // A sensor that still answers at its own address has not reset, so it still ranges.
    for (size_t i = 0; i < 2; ++i) {
        if (!running_[i] && answers(ADDRESSES[i])) {
            running_[i] = true;
            fails_in_row_[i] = 0;
        }
    }
    start_stopped_sensors();
    for (size_t i = 0; i < 2; ++i) {
        if (running_[i] && !was_running[i] && health_[i].recoveries < UINT16_MAX) {
            ++health_[i].recoveries;
        }
    }
}

EdgeReading reading(Side side) {
    return readings_[index_of(side)];
}

EdgeHealth health(Side side) {
    return health_[index_of(side)];
}

}  // namespace edge_sensors
