#include "buttons.h"

#include "pins.h"

namespace buttons {

void begin() {
    pinMode(pins::BUTTON_ADC, INPUT);
}

uint16_t raw() {
    return analogRead(pins::BUTTON_ADC);
}

uint32_t millivolts() {
    return analogReadMilliVolts(pins::BUTTON_ADC);
}

}  // namespace buttons
