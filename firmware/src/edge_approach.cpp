#include "edge_approach.h"

#include "config.h"
#include "display.h"
#include "edge_sensors.h"
#include "emotion.h"
#include "safety.h"
#include "tracks.h"

namespace {

using edge_approach::State;

enum class Outcome : uint8_t { DONE, TIMEOUT, SQUARE_TIMEOUT, SENSOR, INTERLOCK, STOPPED };

constexpr Side SIDES[2] = {Side::LEFT, Side::RIGHT};
constexpr char SIDE_NAMES[2] = {'L', 'R'};

State state_ = State::IDLE;
int8_t duty_ = 0;
bool stopped_[2] = {false, false};
char first_side_ = '-';  // 'L', 'R', 'B' (both in the same tick) or '-' (none yet)
uint32_t start_ms_ = 0;  // STARTING: end of the display draw. Later: start of the tracks
uint32_t first_stop_ms_ = 0;

void print_outcome(Outcome outcome) {
    switch (outcome) {
        case Outcome::DONE:
            Serial.print(F("done"));
            break;
        case Outcome::TIMEOUT:
            Serial.print(F("timeout"));
            break;
        case Outcome::SQUARE_TIMEOUT:
            Serial.print(F("square_timeout"));
            break;
        case Outcome::SENSOR:
            Serial.print(F("sensor"));
            break;
        case Outcome::INTERLOCK:
            Serial.print(F("interlock"));
            break;
        case Outcome::STOPPED:
            Serial.print(F("stopped"));
            break;
    }
}

// Stops the tracks, prints the result line and draws the state.
void finish(Outcome outcome, uint32_t now_ms) {
    tracks::stop();
    uint32_t total_ms = 0;
    if (state_ == State::DRIVING || state_ == State::SQUARING) {
        total_ms = now_ms - start_ms_;
    }
    uint32_t first_ms = 0;
    if (first_side_ != '-') {
        first_ms = first_stop_ms_ - start_ms_;
    }
    state_ = outcome == Outcome::DONE ? State::DONE : State::ABORTED;

    Serial.print(F("result,"));
    print_outcome(outcome);
    Serial.print(',');
    Serial.print(duty_);
    Serial.print(',');
    Serial.print(first_side_);
    Serial.print(',');
    Serial.print(first_ms);
    Serial.print(',');
    Serial.print(total_ms);
    Serial.print(',');
    Serial.print(tracks::counts(Side::LEFT));
    Serial.print(',');
    Serial.println(tracks::counts(Side::RIGHT));

    // Draw only now that the tracks have stopped: drawing blocks the loop (display.h).
    Emotion face = Emotion::SCARED;
    if (outcome == Outcome::DONE) {
        face = Emotion::HAPPY;
    } else if (outcome == Outcome::STOPPED) {
        face = Emotion::NEUTRAL;
    }
    display::show_text(emotion::name(face));
}

// The display draw in start() blocked the loop, so the readings are old. Waits for a reading
// from each sensor that was taken after the draw.
void update_starting(uint32_t now_ms) {
    bool fresh = true;
    for (const Side side : SIDES) {
        const EdgeReading r = edge_sensors::reading(side);
        fresh = fresh && r.valid && static_cast<int32_t>(r.time_ms - start_ms_) >= 0;
    }
    if (fresh) {
        tracks::reset_counts();
        start_ms_ = now_ms;
        state_ = State::DRIVING;
    } else if (now_ms - start_ms_ > config::EDGE_STALE_MS) {
        finish(Outcome::SENSOR, now_ms);
    }
}

void update_driving(uint32_t now_ms) {
    if (!tracks::driver_enabled()) {  // `en 0` during a run
        finish(Outcome::STOPPED, now_ms);
        return;
    }
    if (now_ms - start_ms_ > config::APPROACH_MAX_MS) {
        finish(Outcome::TIMEOUT, now_ms);
        return;
    }
    if (state_ == State::SQUARING && now_ms - first_stop_ms_ > config::SQUARE_MAX_MS) {
        finish(Outcome::SQUARE_TIMEOUT, now_ms);
        return;
    }

    const bool was_driving = state_ == State::DRIVING;
    uint8_t new_stops = 0;
    for (size_t i = 0; i < 2; ++i) {
        if (stopped_[i]) {
            continue;
        }
        const Side side = SIDES[i];
        // A sensor without fresh data is a fault, not an edge.
        if (!safety::sensor_live(side)) {
            finish(Outcome::SENSOR, now_ms);
            return;
        }
        if (safety::sees_table(side)) {
            if (!safety::forward_allowed(side)) {
                finish(Outcome::INTERLOCK, now_ms);
                return;
            }
            tracks::run(side, duty_, config::TRACK_LEASE_MS);  // renews the lease
            continue;
        }
        // This side has reached the edge: stop its track for the rest of the run.
        tracks::run(side, 0, 0);
        stopped_[i] = true;
        ++new_stops;
        if (first_side_ == '-') {
            first_side_ = SIDE_NAMES[i];
            first_stop_ms_ = now_ms;
            state_ = State::SQUARING;
        }
    }
    if (was_driving && new_stops == 2) {
        first_side_ = 'B';
    }
    if (stopped_[0] && stopped_[1]) {
        finish(Outcome::DONE, now_ms);
    }
}

}  // namespace

namespace edge_approach {

bool start(int8_t duty_percent) {
    if (active()) {
        Serial.println(F("refused: a run is active, `stop` first"));
        return false;
    }
    if (duty_percent < 1 || duty_percent > config::TRACK_MAX_DUTY_PCT) {
        Serial.print(F("refused: duty must be 1.."));
        Serial.print(config::TRACK_MAX_DUTY_PCT);
        Serial.println(F(" %"));
        return false;
    }
    if (!tracks::driver_enabled()) {
        Serial.println(F("refused: drivers off, `en 1` first"));
        return false;
    }
    // The display draw below blocks the loop, so nothing may be moving.
    if (tracks::duty(Side::LEFT) != 0 || tracks::duty(Side::RIGHT) != 0) {
        Serial.println(F("refused: tracks are moving"));
        return false;
    }
    if (!safety::forward_allowed(Side::LEFT) || !safety::forward_allowed(Side::RIGHT)) {
        Serial.println(
            F("refused: both sensors must see the table, arm servo off, actuator still"));
        return false;
    }

    duty_ = duty_percent;
    stopped_[0] = false;
    stopped_[1] = false;
    first_side_ = '-';
    first_stop_ms_ = 0;
    Serial.print(F("approach at "));
    Serial.print(duty_);
    Serial.println(F(" %"));
    Serial.println(F("result,outcome,duty_pct,first_side,t_first_ms,t_total_ms,enc_l,enc_r"));

    // The tracks are still stopped, so the blocking draw is safe here.
    display::show_text(emotion::name(Emotion::WORKING));
    state_ = State::STARTING;
    start_ms_ = millis();
    return true;
}

void abort() {
    if (active()) {
        finish(Outcome::STOPPED, millis());
    }
}

void update(uint32_t now_ms) {
    switch (state_) {
        case State::STARTING:
            update_starting(now_ms);
            break;
        case State::DRIVING:
        case State::SQUARING:
            update_driving(now_ms);
            break;
        default:
            break;
    }
}

bool active() {
    return state_ == State::STARTING || state_ == State::DRIVING || state_ == State::SQUARING;
}

State state() {
    return state_;
}

}  // namespace edge_approach
