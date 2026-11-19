// Serial command console over USB for bench bring-up (step 1). Type `help` for the commands.
#pragma once

#include <Arduino.h>

namespace console {

// Prints the bench-only banner and the command list.
void begin();

// Reads console input without waiting, runs complete commands and prints the status stream.
// Call in every loop iteration.
void update(uint32_t now_ms);

}  // namespace console
