// The face display: 2.8" ILI9341 SPI TFT, 240 x 320 pixels (D009). Step 1 shows a test
// pattern and one line of text. The faces come in step 3.
#pragma once

#include <Arduino.h>

namespace display {

// Starts SPI without a MISO pin (D12 is the IMU input) and initializes the display with a
// software reset, since its RESET pin is tied high. The display cannot be read back (MISO is
// not connected), so success is checked by eye.
void begin();

// Fills the screen red, green, blue and white, then shows a title.
void show_test_pattern();

// Clears the screen and shows `text` in large letters.
void show_text(const char* text);

}  // namespace display
