#include "tracks.h"

#include "config.h"
#include "pins.h"
#include "safety.h"

namespace {

struct Track {
    uint8_t in1;  // PWM pin
    uint8_t in2;  // digital pin
    bool reversed;
    int8_t duty;  // percent, -100 to 100
    bool running;
    uint32_t stop_at_ms;
};

Track tracks_[2] = {
    {pins::LEFT_IN1, pins::LEFT_IN2, config::LEFT_MOTOR_REVERSED, 0, false, 0},
    {pins::RIGHT_IN1, pins::RIGHT_IN2, config::RIGHT_MOTOR_REVERSED, 0, false, 0},
};

// Written by the encoder interrupts, read by the main loop with interrupts off, since a
// 32-bit access takes several instructions on the 8-bit AVR.
volatile int32_t counts_[2] = {0, 0};
volatile int8_t direction_[2] = {1, 1};
bool driver_enabled_ = false;

void on_left_edge() {
    counts_[0] = counts_[0] + direction_[0];
}
void on_right_edge() {
    counts_[1] = counts_[1] + direction_[1];
}

size_t index_of(Side side) {
    return static_cast<size_t>(side);
}

// The edge gate and the reverse rule (safety.h). Zero duty is always allowed.
bool allowed(Side side, int8_t duty_percent) {
    if (duty_percent > 0) {
        return safety::forward_allowed(side);
    }
    return duty_percent == 0 || safety::REVERSE_ALLOWED;
}

// Only IN1 has PWM (D020). With the enable pin high (L293D datasheet, Table 3):
// - forward: IN2 low, IN1 at `level`. Between pulses both outputs are low: fast motor stop.
// - reverse: IN2 high, IN1 at 255 - `level`. While IN1 is high, both outputs are high: fast
//   motor stop. While IN1 is low, the motor runs in reverse, so the drive time is `level`.
// analogWrite() turns 0 and 255 into plain digitalWrite() calls (wiring_analog.c).
void write_duty(const Track& track, int8_t duty_percent) {
    const int16_t signed_duty = track.reversed ? -duty_percent : duty_percent;
    const uint16_t magnitude = static_cast<uint16_t>(signed_duty < 0 ? -signed_duty : signed_duty);
    // Percent to 0..255, rounded to the nearest step.
    const uint8_t level = static_cast<uint8_t>((magnitude * config::MOTOR_PWM_MAX + 50u) / 100u);
    if (signed_duty >= 0) {
        digitalWrite(track.in2, LOW);
        analogWrite(track.in1, level);
    } else {
        digitalWrite(track.in2, HIGH);
        analogWrite(track.in1, config::MOTOR_PWM_MAX - level);
    }
}

}  // namespace

namespace tracks {

void begin() {
    pinMode(pins::MOTOR_ENABLE, OUTPUT);
    digitalWrite(pins::MOTOR_ENABLE, LOW);
    driver_enabled_ = false;

    for (const Track& track : tracks_) {
        digitalWrite(track.in1, LOW);
        digitalWrite(track.in2, LOW);
        pinMode(track.in1, OUTPUT);
        pinMode(track.in2, OUTPUT);
    }

    // The core starts Timer4 in phase and frequency correct PWM (wiring.c, init()), about
    // 490 Hz. WGM41:40 = 00 selects fast PWM with TOP = OCR4C = 0xFF (ATmega32U4 datasheet
    // 7766J, Table 15-18), so D6 runs at 16 MHz / 64 / 256 = 977 Hz, like D11 on Timer0.
    TCCR4D &= static_cast<uint8_t>(~(_BV(WGM41) | _BV(WGM40)));

    // The internal pull-up holds phase A at 5 V (20-50 kOhm, ATmega32U4 datasheet table 29-1,
    // D020). The encoders are open collector.
    pinMode(pins::ENC_LEFT_A, INPUT_PULLUP);
    pinMode(pins::ENC_RIGHT_A, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pins::ENC_LEFT_A), on_left_edge, RISING);
    attachInterrupt(digitalPinToInterrupt(pins::ENC_RIGHT_A), on_right_edge, RISING);
}

void set_driver_enable(bool on) {
    digitalWrite(pins::MOTOR_ENABLE, on ? HIGH : LOW);
    driver_enabled_ = on;
}

bool driver_enabled() {
    return driver_enabled_;
}

uint32_t run(Side side, int8_t duty_percent, uint32_t run_ms) {
    const size_t i = index_of(side);
    Track& track = tracks_[i];
    constexpr int8_t CAP = config::TRACK_MAX_DUTY_PCT;
    int8_t clamped = constrain(duty_percent, static_cast<int8_t>(-CAP), CAP);
    uint32_t used_ms = min(run_ms, config::TRACK_MAX_RUN_MS);
    if (!allowed(side, clamped)) {
        // A refused command also stops the track, in case it was running.
        clamped = 0;
        used_ms = 0;
    }

    if (clamped > 0) {
        direction_[i] = 1;
    } else if (clamped < 0) {
        direction_[i] = -1;
    }
    track.duty = clamped;
    track.running = clamped != 0;
    track.stop_at_ms = millis() + used_ms;
    write_duty(track, clamped);
    safety::kick_watchdog();  // arms the watchdog at once when the track starts
    return used_ms;
}

void stop() {
    for (Track& track : tracks_) {
        track.duty = 0;
        track.running = false;
        write_duty(track, 0);
    }
    safety::kick_watchdog();  // disarms it, so a display draw after a stop does not trip it
}

void update(uint32_t now_ms) {
    for (size_t i = 0; i < 2; ++i) {
        Track& track = tracks_[i];
        const bool expired = static_cast<int32_t>(now_ms - track.stop_at_ms) >= 0;
        if (track.running && (expired || !allowed(static_cast<Side>(i), track.duty))) {
            track.duty = 0;
            track.running = false;
            write_duty(track, 0);
        }
    }
}

int8_t duty(Side side) {
    return tracks_[index_of(side)].duty;
}

int32_t counts(Side side) {
    const uint8_t sreg = SREG;
    cli();
    const int32_t value = counts_[index_of(side)];
    SREG = sreg;
    return value;
}

void reset_counts() {
    const uint8_t sreg = SREG;
    cli();
    counts_[0] = 0;
    counts_[1] = 0;
    SREG = sreg;
}

}  // namespace tracks
