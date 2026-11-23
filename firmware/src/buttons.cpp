#include "buttons.h"

#include "pins.h"

namespace buttons {

void begin() {
    pinMode(pins::BUTTON_ADC, INPUT);
}

uint16_t raw() {
    return static_cast<uint16_t>(analogRead(pins::BUTTON_ADC));
}

}  // namespace buttons
