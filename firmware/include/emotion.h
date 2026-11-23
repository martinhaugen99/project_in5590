// The seven states of the face (D009), shared by the behavior layer and the display
// (CLAUDE.md, "Display & emotions"). Step 2 draws the name of the state as text.
#pragma once

#include <Arduino.h>

enum class Emotion : uint8_t { NEUTRAL, WORKING, HAPPY, SAD, SURPRISED, SCARED, ANGRY };

namespace emotion {

// The name drawn on the display.
inline const char* name(Emotion state) {
    switch (state) {
        case Emotion::NEUTRAL:
            return "Neutral";
        case Emotion::WORKING:
            return "Working";
        case Emotion::HAPPY:
            return "Happy";
        case Emotion::SAD:
            return "Sad";
        case Emotion::SURPRISED:
            return "Surprised";
        case Emotion::SCARED:
            return "Scared";
        case Emotion::ANGRY:
            return "Angry";
    }
    return "";
}

}  // namespace emotion
