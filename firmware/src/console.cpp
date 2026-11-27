#include "console.h"

#include <ctype.h>
#include <string.h>

#include "actuator.h"
#include "config.h"
#include "display.h"
#include "edge_approach.h"
#include "edge_sensors.h"
#include "safety.h"
#include "servos.h"
#include "tracks.h"

namespace {

char line_[config::CONSOLE_LINE_MAX + 1] = {};
uint8_t length_ = 0;
bool stream_ = false;
uint32_t last_stream_ms_ = 0;

// The AVR core has no Serial.printf, and string literals live in RAM unless they are wrapped
// in F() or PROGMEM. The help text stays in flash.
const char HELP[] PROGMEM =
    "Step 2: edge gate on, reverse refused.\n"
    " stream on|off | en 0|1 | stop\n"
    " start | approach [duty %]\n"
    " drive L|R|B <duty %> <ms<=3000>\n"
    " enc reset | tof\n"
    " servo brush <500..2500 us>|off\n"
    " servo bucket <750..2250 us>|off (arms)\n"
    " act extend|retract <ms<=500>\n"
    " face <text>\n";

void print_help() {
    Serial.print(reinterpret_cast<const __FlashStringHelper*>(HELP));
}

// Parses a whole token as a base-10 integer with an optional minus sign. Returns false for
// empty or partial numbers, or more than 9 digits. Replaces strtol(), which takes about
// 0.55 KB of flash.
bool parse_long(const char* token, long& value) {
    if (token == nullptr) {
        return false;
    }
    const bool negative = *token == '-';
    if (negative) {
        ++token;
    }
    long result = 0;
    uint8_t digits = 0;
    for (; *token != '\0'; ++token) {
        if (!isdigit(static_cast<unsigned char>(*token)) || ++digits > 9) {
            return false;
        }
        result = result * 10 + (*token - '0');
    }
    if (digits == 0) {
        return false;
    }
    value = negative ? -result : result;
    return true;
}

bool equals(const char* token, const char* text) {
    return token != nullptr && strcmp(token, text) == 0;
}

void print_tof(Side side) {
    const EdgeReading r = edge_sensors::reading(side);
    if (!edge_sensors::ok(side)) {
        Serial.print(F("FAULT"));
    } else if (!r.valid) {
        Serial.print(F("no data"));
    } else {
        Serial.print(r.distance_mm);
        Serial.print(F(" mm (status "));
        Serial.print(r.range_status);
        Serial.print(F(", "));
        Serial.print(millis() - r.time_ms);
        Serial.print(F(" ms ago)"));
    }
    Serial.print(safety::sees_table(side) ? F(" table") : F(" NO TABLE"));
    // Errors since the reset, the code of the last one (edge_sensors.h) and the recoveries.
    const EdgeHealth h = edge_sensors::health(side);
    Serial.print(F(", err "));
    Serial.print(h.errors);
    Serial.print(F(" code "));
    Serial.print(h.last_code);
    Serial.print(F(" rec "));
    Serial.print(h.recoveries);
}

void print_tof_line() {
    Serial.print(F("tof L "));
    print_tof(Side::LEFT);
    Serial.print(F(" | R "));
    print_tof(Side::RIGHT);
    Serial.println();
}

void print_stream_header() {
    Serial.println(
        F("t_ms,drivers,duty_l_pct,duty_r_pct,enc_l,enc_r,tof_l_mm,tof_l_status,tof_r_mm,tof_r_"
          "status,"
          "act_raw,table_l,table_r,run"));
}

void print_stream_line(uint32_t now_ms) {
    const EdgeReading left = edge_sensors::reading(Side::LEFT);
    const EdgeReading right = edge_sensors::reading(Side::RIGHT);
    Serial.print(now_ms);
    Serial.print(',');
    Serial.print(tracks::driver_enabled() ? 1 : 0);
    Serial.print(',');
    Serial.print(tracks::duty(Side::LEFT));
    Serial.print(',');
    Serial.print(tracks::duty(Side::RIGHT));
    Serial.print(',');
    Serial.print(tracks::counts(Side::LEFT));
    Serial.print(',');
    Serial.print(tracks::counts(Side::RIGHT));
    Serial.print(',');
    Serial.print(left.distance_mm);
    Serial.print(',');
    Serial.print(left.range_status);
    Serial.print(',');
    Serial.print(right.distance_mm);
    Serial.print(',');
    Serial.print(right.range_status);
    Serial.print(',');
    Serial.print(actuator::feedback_raw());
    Serial.print(',');
    Serial.print(safety::sees_table(Side::LEFT) ? 1 : 0);
    Serial.print(',');
    Serial.print(safety::sees_table(Side::RIGHT) ? 1 : 0);
    Serial.print(',');
    Serial.println(static_cast<int>(edge_approach::state()));
}

void warn_if_disabled() {
    if (!tracks::driver_enabled()) {
        Serial.println(F("note: drivers are disabled, nothing moves until `en 1`"));
    }
}

void cmd_drive(const char* side_token, const char* duty_token, const char* ms_token) {
    long duty_percent = 0;
    long run_ms = 0;
    const char side = side_token != nullptr && strlen(side_token) == 1
                          ? static_cast<char>(toupper(side_token[0]))
                          : '\0';
    if ((side != 'L' && side != 'R' && side != 'B') || !parse_long(duty_token, duty_percent) ||
        !parse_long(ms_token, run_ms) || duty_percent < -100 || duty_percent > 100 || run_ms < 0) {
        Serial.println(F("usage: drive L|R|B <duty -100..100> <ms>"));
        return;
    }
    if (edge_approach::active()) {
        Serial.println(F("refused: approach running, `stop` first"));
        return;
    }
    const int8_t duty = static_cast<int8_t>(duty_percent);
    uint32_t used_ms = 0;
    bool refused = false;
    if (side == 'L' || side == 'B') {
        used_ms = tracks::run(Side::LEFT, duty, static_cast<uint32_t>(run_ms));
        refused = refused || (duty != 0 && tracks::duty(Side::LEFT) == 0);
    }
    if (side == 'R' || side == 'B') {
        used_ms = tracks::run(Side::RIGHT, duty, static_cast<uint32_t>(run_ms));
        refused = refused || (duty != 0 && tracks::duty(Side::RIGHT) == 0);
    }
    if (refused) {
        // With `B`, one side may have been taken. Stop both, so nothing drives half a command.
        tracks::stop();
        Serial.println(duty < 0 ? F("refused: reverse is off in step 2")
                                : F("refused: edge gate or interlock, see `tof`"));
        return;
    }
    Serial.print(F("drive "));
    Serial.print(side);
    Serial.print(' ');
    Serial.print(tracks::duty(side == 'R' ? Side::RIGHT : Side::LEFT));  // after the speed cap
    Serial.print(F(" % for "));
    Serial.print(used_ms);
    Serial.println(F(" ms"));
    warn_if_disabled();
}

void cmd_servo(const char* name_token, const char* value_token) {
    ServoId id;
    if (equals(name_token, "brush")) {
        id = ServoId::BRUSH;
    } else if (equals(name_token, "bucket")) {
        id = ServoId::BUCKET;
    } else {
        Serial.println(F("usage: servo brush|bucket <us>|off"));
        return;
    }
    if (equals(value_token, "off")) {
        servos::release(id);
        Serial.print(F("servo "));
        Serial.print(name_token);
        Serial.println(F(" off"));
        return;
    }
    long pulse = 0;
    if (!parse_long(value_token, pulse) || pulse < 0 || pulse > 0xFFFF) {
        Serial.println(F("usage: servo brush|bucket <us>|off"));
        return;
    }
    const uint16_t used = servos::set_pulse_us(id, static_cast<uint16_t>(pulse));
    if (used == 0) {
        Serial.println(F("refused: arms only with both tracks stopped"));
        return;
    }
    Serial.print(F("servo "));
    Serial.print(name_token);
    Serial.print(' ');
    Serial.print(used);
    Serial.println(F(" us"));
}

void cmd_act(const char* motion_token, const char* ms_token) {
    long run_ms = 0;
    actuator::Motion motion;
    if (equals(motion_token, "extend")) {
        motion = actuator::Motion::EXTEND;
    } else if (equals(motion_token, "retract")) {
        motion = actuator::Motion::RETRACT;
    } else {
        Serial.println(F("usage: act extend|retract <ms>"));
        return;
    }
    if (!parse_long(ms_token, run_ms) || run_ms <= 0) {
        Serial.println(F("usage: act extend|retract <ms>"));
        return;
    }
    const uint16_t before = actuator::feedback_raw();
    const uint32_t used_ms = actuator::jog(motion, static_cast<uint32_t>(run_ms));
    if (used_ms == 0) {
        Serial.println(F("refused: a track is running"));
        return;
    }
    Serial.print(F("act "));
    Serial.print(motion_token);
    Serial.print(F(" for "));
    Serial.print(used_ms);
    Serial.print(F(" ms, feedback before "));
    Serial.print(before);
    Serial.println(F(" raw (`stream on` shows the move)"));
    warn_if_disabled();
}

void run_command(char* line) {
    char* command = strtok(line, " ");
    if (command == nullptr) {
        return;
    }
    char* arg1 = strtok(nullptr, " ");
    char* arg2 = strtok(nullptr, " ");
    char* arg3 = strtok(nullptr, " ");

    if (equals(command, "help")) {
        print_help();
    } else if (equals(command, "stream")) {
        stream_ = equals(arg1, "on");
        if (stream_) {
            print_stream_header();
        }
    } else if (equals(command, "en")) {
        const bool on = equals(arg1, "1");
        tracks::set_driver_enable(on);
        Serial.print(F("drivers "));
        Serial.println(on ? F("ON") : F("off"));
    } else if (equals(command, "stop")) {
        edge_approach::abort();
        tracks::stop();
        actuator::stop();
        tracks::set_driver_enable(false);
        Serial.println(F("stopped, drivers off (servos keep their pulses, use `servo ... off`)"));
    } else if (equals(command, "start")) {
        // The start trigger: the robot has no buttons (D031). Until the dump cycle is written, it
        // runs the edge approach at the default duty.
        edge_approach::start(config::APPROACH_DEFAULT_DUTY_PCT);
    } else if (equals(command, "approach")) {
        long duty = config::APPROACH_DEFAULT_DUTY_PCT;
        if (arg1 != nullptr && !parse_long(arg1, duty)) {
            Serial.println(F("usage: approach [duty %]"));
        } else {
            // Out-of-range values become 0, which start() refuses with the valid range.
            edge_approach::start(duty >= 1 && duty <= 100 ? static_cast<int8_t>(duty) : 0);
        }
    } else if (equals(command, "drive")) {
        cmd_drive(arg1, arg2, arg3);
    } else if (equals(command, "enc") && equals(arg1, "reset")) {
        tracks::reset_counts();
        Serial.println(F("encoder counts reset"));
    } else if (equals(command, "tof")) {
        print_tof_line();
    } else if (equals(command, "servo")) {
        cmd_servo(arg1, arg2);
    } else if (equals(command, "act")) {
        cmd_act(arg1, arg2);
    } else if (equals(command, "face")) {
        if (arg1 != nullptr) {
            display::show_text(arg1);
        } else {
            Serial.println(F("usage: face <text>"));
        }
    } else {
        Serial.print(F("unknown command: "));
        Serial.print(command);
        Serial.println(F(" (type `help`)"));
    }
}

}  // namespace

namespace console {

void begin() {
    Serial.println();
    Serial.println(F("M.A.T.S firmware, step 2: edge approach (Arduino Leonardo)"));
    Serial.println(F("Edge gate on: a track drives forward only while its sensor sees the table."));
    print_help();
}

void update(uint32_t now_ms) {
    while (Serial.available() > 0) {
        const char c = static_cast<char>(Serial.read());
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            line_[length_] = '\0';
            run_command(line_);
            length_ = 0;
            continue;
        }
        if (length_ < config::CONSOLE_LINE_MAX) {
            line_[length_++] = c;
        }
    }

    if (stream_ && now_ms - last_stream_ms_ >= config::STREAM_PERIOD_MS) {
        last_stream_ms_ = now_ms;
        print_stream_line(now_ms);
    }
}

}  // namespace console
