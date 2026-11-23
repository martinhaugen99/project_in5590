#include "display.h"

#include <Adafruit_ILI9341.h>
#include <SPI.h>

#include "pins.h"

namespace {

// Hardware SPI on the default pins (ICSP header). RESET is tied to 3V3, so rst = -1 and
// Adafruit_ILI9341::begin() sends a software reset.
Adafruit_ILI9341 tft_(pins::TFT_CS, pins::TFT_DC, -1);

}  // namespace

namespace display {

void begin() {
    tft_.begin();
}

void show_text(const char* text) {
    tft_.fillScreen(ILI9341_BLACK);
    tft_.setTextColor(ILI9341_WHITE);
    tft_.setTextSize(3);
    tft_.setCursor(10, 10);
    tft_.print(text);
}

}  // namespace display
