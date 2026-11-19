#include "tracks.h"

#include "config.h"
#include "pins.h"

namespace {

struct Track {
    uint8_t in1;
    uint8_t in2;
    bool reversed;
    float duty;
    bool running;
    uint32_t stop_at_ms;
};

Track tracks_[2] = {
    {pins::LEFT_IN1, pins::LEFT_IN2, config::LEFT_MOTOR_REVERSED, 0.0f, false, 0},
    {pins::RIGHT_IN1, pins::RIGHT_IN2, config::RIGHT_MOTOR_REVERSED, 0.0f, false, 0},
};

// Written by the encoder interrupts, read by the main loop. 32-bit reads are atomic.
volatile int32_t counts_[2] = {0, 0};
volatile int8_t direction_[2] = {1, 1};
bool driver_enabled_ = false;

void IRAM_ATTR on_left_edge() {
    counts_[0] = counts_[0] + direction_[0];
}
void IRAM_ATTR on_right_edge() {
    counts_[1] = counts_[1] + direction_[1];
}

size_t index_of(Side side) {
    return static_cast<size_t>(side);
}

// Sign-magnitude drive: PWM on one input and the other input low. Between pulses both outputs
// are low, which brakes the motor (L293D datasheet, Table 3: "Fast motor stop").
void write_duty(const Track& track, float duty) {
    const float signed_duty = track.reversed ? -duty : duty;
    const uint32_t level = static_cast<uint32_t>(
        fabsf(signed_duty) * static_cast<float>(config::MOTOR_PWM_MAX) + 0.5f);
    if (signed_duty >= 0.0f) {
        ledcWrite(track.in2, 0);
        ledcWrite(track.in1, level);
    } else {
        ledcWrite(track.in1, 0);
        ledcWrite(track.in2, level);
    }
}

}  // namespace

namespace tracks {

bool begin() {
    pinMode(pins::MOTOR_ENABLE, OUTPUT);
    digitalWrite(pins::MOTOR_ENABLE, LOW);
    driver_enabled_ = false;

    bool ok = true;
    for (const Track& track : tracks_) {
        ok = ledcAttach(track.in1, config::MOTOR_PWM_HZ, config::MOTOR_PWM_BITS) && ok;
        ok = ledcAttach(track.in2, config::MOTOR_PWM_HZ, config::MOTOR_PWM_BITS) && ok;
        ledcWrite(track.in1, 0);
        ledcWrite(track.in2, 0);
    }

    // D005 asks for external pull-ups to 3.3 V on the open-collector encoder outputs. The
    // internal pull-up is only a backup.
    pinMode(pins::ENC_LEFT_A, INPUT_PULLUP);
    pinMode(pins::ENC_RIGHT_A, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pins::ENC_LEFT_A), on_left_edge, RISING);
    attachInterrupt(digitalPinToInterrupt(pins::ENC_RIGHT_A), on_right_edge, RISING);
    return ok;
}

void set_driver_enable(bool on) {
    digitalWrite(pins::MOTOR_ENABLE, on ? HIGH : LOW);
    driver_enabled_ = on;
}

bool driver_enabled() {
    return driver_enabled_;
}

uint32_t run(Side side, float duty, uint32_t run_ms) {
    const size_t i = index_of(side);
    Track& track = tracks_[i];
    const float clamped = constrain(duty, -1.0f, 1.0f);
    const uint32_t used_ms = min(run_ms, config::TRACK_MAX_RUN_MS);

    if (clamped > 0.0f) {
        direction_[i] = 1;
    } else if (clamped < 0.0f) {
        direction_[i] = -1;
    }
    track.duty = clamped;
    track.running = clamped != 0.0f;
    track.stop_at_ms = millis() + used_ms;
    write_duty(track, clamped);
    return used_ms;
}

void stop() {
    for (Track& track : tracks_) {
        track.duty = 0.0f;
        track.running = false;
        write_duty(track, 0.0f);
    }
}

void update(uint32_t now_ms) {
    for (Track& track : tracks_) {
        if (track.running && static_cast<int32_t>(now_ms - track.stop_at_ms) >= 0) {
            track.duty = 0.0f;
            track.running = false;
            write_duty(track, 0.0f);
        }
    }
}

float duty(Side side) {
    return tracks_[index_of(side)].duty;
}

int32_t counts(Side side) {
    return counts_[index_of(side)];
}

void reset_counts() {
    counts_[0] = 0;
    counts_[1] = 0;
}

}  // namespace tracks
