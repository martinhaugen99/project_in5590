// The face display: 2.8" ILI9341 SPI TFT, 240 x 320 pixels (D009). It shows one line of text,
// such as the name of a state (emotion.h). The faces come later.
//
// Drawing blocks the loop: a full screen is 240 x 320 x 16 bits, at most 8 MHz SPI, so at least
// 154 ms (D020). Draw only while nothing moves.
#pragma once

#include <Arduino.h>

namespace display {

// Starts the hardware SPI on the ICSP header and initializes the display with a software
// reset, since its RESET pin is tied high. The display cannot be read back (MISO is not
// connected), so success is checked by eye.
void begin();

// Clears the screen and shows `text` in large letters.
void show_text(const char* text);

}  // namespace display
