#include "display.h"

#include <Adafruit_ILI9341.h>
#include <SPI.h>

#include "pins.h"

namespace {

// RESET is tied to 3V3, so rst = -1 and Adafruit_ILI9341::begin() sends a software reset.
Adafruit_ILI9341 tft_(&SPI, pins::TFT_DC, pins::TFT_CS, -1);

constexpr uint16_t TEST_COLORS[] = {ILI9341_RED, ILI9341_GREEN, ILI9341_BLUE, ILI9341_WHITE};
constexpr uint32_t TEST_COLOR_MS = 300;

}  // namespace

namespace display {

void begin() {
    // Start SPI here with MISO = -1. Adafruit's initSPI() calls SPI.begin() with no pins,
    // which returns early once the bus runs (SPI.cpp), so D12 stays free for the IMU.
    SPI.begin(pins::TFT_SCK, -1, pins::TFT_MOSI, -1);
    tft_.begin();
}

void show_test_pattern() {
    for (const uint16_t color : TEST_COLORS) {
        tft_.fillScreen(color);
        delay(TEST_COLOR_MS);
    }
    show_text("M.A.T.S");
}

void show_text(const char* text) {
    tft_.fillScreen(ILI9341_BLACK);
    tft_.setTextColor(ILI9341_WHITE);
    tft_.setTextSize(3);
    tft_.setCursor(10, 10);
    tft_.print(text);
}

}  // namespace display
