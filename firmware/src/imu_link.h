// Serial link from the OpenLog Artemis IMU (D010): its TX pin into D12 (Serial1 RX). The
// OpenLog Artemis sends its logged data on TX only when "Output serial data to TX pin" is on
// in its serial logging menu, at the baud rate set in that menu. Step 1 passes the raw lines
// through to the console. Parsing comes in step 2.
#pragma once

#include <Arduino.h>

namespace imu_link {

// Starts Serial1 on D12 with no TX pin, at the baud rate set on the OpenLog Artemis.
void start(uint32_t baud);
void stop();
bool running();

// Prints each complete line to `out` while echo is on.
void set_echo(bool on);

// Reads the bytes that have arrived and assembles them into lines. Call in every loop
// iteration.
void update(Print& out, uint32_t now_ms);

uint32_t line_count();

// millis() of the last complete line, or 0 if none has arrived.
uint32_t last_line_ms();

}  // namespace imu_link
