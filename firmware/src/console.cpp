#include "console.h"

#include <stdlib.h>
#include <string.h>

#include "actuator.h"
#include "buttons.h"
#include "config.h"
#include "display.h"
#include "edge_sensors.h"
#include "imu_link.h"
#include "servos.h"
#include "tracks.h"

namespace {

char line_[config::CONSOLE_LINE_MAX + 1] = {};
size_t length_ = 0;
bool stream_ = false;
uint32_t last_stream_ms_ = 0;

constexpr const char* HELP =
    "Commands (bench only, tracks off the ground):\n"
    "  help                          this list\n"
    "  status                        one line with every reading\n"
    "  stream on|off                 CSV status lines every 100 ms\n"
    "  en 0|1                        disable/enable both L293D chips (tracks and actuator)\n"
    "  stop                          stop tracks and actuator, disable the drivers\n"
    "  drive L|R|B <duty> <ms>       duty -100..100 %, ms <= 3000, stops by itself\n"
    "  enc reset                     zero the encoder counts\n"
    "  tof                           distances of both edge sensors\n"
    "  servo brush|bucket <us>|off   pulse 500..2500 us, or no pulses\n"
    "  act extend|retract <ms>       jog the actuator, ms <= 500, prints the feedback\n"
    "  btn                           raw level of the button ladder\n"
    "  imu on <baud> | imu off       echo the OpenLog Artemis lines\n"
    "  face test | face <text>       display test pattern or text\n";

// Parses a whole token as a base-10 integer. Returns false for empty or partial numbers.
bool parse_long(const char* token, long& value) {
    if (token == nullptr || *token == '\0') {
        return false;
    }
    char* end = nullptr;
    value = strtol(token, &end, 10);
    return *end == '\0';
}

void print_tof(Side side) {
    const EdgeReading r = edge_sensors::reading(side);
    if (!edge_sensors::ok(side)) {
        Serial.print("not started");
    } else if (!r.valid) {
        Serial.print("no data");
    } else {
        Serial.printf("%u mm (status %u, %lu ms ago)", r.distance_mm, r.range_status,
                      static_cast<unsigned long>(millis() - r.time_ms));
    }
}

void print_status() {
    Serial.printf("drivers %s | duty L %.2f R %.2f | enc L %ld R %ld\n",
                  tracks::driver_enabled() ? "ON" : "off", tracks::duty(Side::LEFT),
                  tracks::duty(Side::RIGHT), static_cast<long>(tracks::counts(Side::LEFT)),
                  static_cast<long>(tracks::counts(Side::RIGHT)));
    Serial.print("tof L ");
    print_tof(Side::LEFT);
    Serial.print(" | R ");
    print_tof(Side::RIGHT);
    Serial.println();
    Serial.printf("act %lu mV (motion %d) | servo brush %lu us, bucket %lu us (0 = off)\n",
                  static_cast<unsigned long>(actuator::feedback_mv()),
                  static_cast<int>(actuator::motion()),
                  static_cast<unsigned long>(servos::pulse_us(ServoId::BRUSH)),
                  static_cast<unsigned long>(servos::pulse_us(ServoId::BUCKET)));
    Serial.printf("btn %u raw, %lu mV | imu %s, %lu lines\n", buttons::raw(),
                  static_cast<unsigned long>(buttons::millivolts()),
                  imu_link::running() ? "on" : "off",
                  static_cast<unsigned long>(imu_link::line_count()));
}

void print_stream_header() {
    Serial.println(
        "t_ms,drivers,duty_l,duty_r,enc_l,enc_r,tof_l_mm,tof_l_status,tof_r_mm,tof_r_status,"
        "act_mv,btn_raw");
}

void print_stream_line(uint32_t now_ms) {
    const EdgeReading left = edge_sensors::reading(Side::LEFT);
    const EdgeReading right = edge_sensors::reading(Side::RIGHT);
    Serial.printf("%lu,%d,%.2f,%.2f,%ld,%ld,%u,%u,%u,%u,%lu,%u\n",
                  static_cast<unsigned long>(now_ms), tracks::driver_enabled() ? 1 : 0,
                  tracks::duty(Side::LEFT), tracks::duty(Side::RIGHT),
                  static_cast<long>(tracks::counts(Side::LEFT)),
                  static_cast<long>(tracks::counts(Side::RIGHT)), left.distance_mm,
                  left.range_status, right.distance_mm, right.range_status,
                  static_cast<unsigned long>(actuator::feedback_mv()), buttons::raw());
}

void warn_if_disabled() {
    if (!tracks::driver_enabled()) {
        Serial.println("note: drivers are disabled, nothing moves until `en 1`");
    }
}

void cmd_drive(char* side_token, char* duty_token, char* ms_token) {
    long duty_percent = 0;
    long run_ms = 0;
    const char side = side_token != nullptr && strlen(side_token) == 1
                          ? static_cast<char>(toupper(side_token[0]))
                          : '\0';
    if ((side != 'L' && side != 'R' && side != 'B') || !parse_long(duty_token, duty_percent) ||
        !parse_long(ms_token, run_ms) || duty_percent < -100 || duty_percent > 100 || run_ms < 0) {
        Serial.println("usage: drive L|R|B <duty -100..100> <ms>");
        return;
    }
    const float duty = static_cast<float>(duty_percent) / 100.0f;
    uint32_t used_ms = 0;
    if (side == 'L' || side == 'B') {
        used_ms = tracks::run(Side::LEFT, duty, static_cast<uint32_t>(run_ms));
    }
    if (side == 'R' || side == 'B') {
        used_ms = tracks::run(Side::RIGHT, duty, static_cast<uint32_t>(run_ms));
    }
    Serial.printf("drive %c %ld %% for %lu ms\n", side, duty_percent,
                  static_cast<unsigned long>(used_ms));
    warn_if_disabled();
}

void cmd_servo(char* name_token, char* value_token) {
    ServoId id;
    if (name_token != nullptr && strcmp(name_token, "brush") == 0) {
        id = ServoId::BRUSH;
    } else if (name_token != nullptr && strcmp(name_token, "bucket") == 0) {
        id = ServoId::BUCKET;
    } else {
        Serial.println("usage: servo brush|bucket <us>|off");
        return;
    }
    if (value_token != nullptr && strcmp(value_token, "off") == 0) {
        servos::release(id);
        Serial.printf("servo %s off\n", name_token);
        return;
    }
    long pulse = 0;
    if (!parse_long(value_token, pulse) || pulse < 0) {
        Serial.println("usage: servo brush|bucket <us>|off");
        return;
    }
    const uint32_t used = servos::set_pulse_us(id, static_cast<uint32_t>(pulse));
    Serial.printf("servo %s %lu us\n", name_token, static_cast<unsigned long>(used));
}

void cmd_act(char* motion_token, char* ms_token) {
    long run_ms = 0;
    actuator::Motion motion;
    if (motion_token != nullptr && strcmp(motion_token, "extend") == 0) {
        motion = actuator::Motion::EXTEND;
    } else if (motion_token != nullptr && strcmp(motion_token, "retract") == 0) {
        motion = actuator::Motion::RETRACT;
    } else {
        Serial.println("usage: act extend|retract <ms>");
        return;
    }
    if (!parse_long(ms_token, run_ms) || run_ms <= 0) {
        Serial.println("usage: act extend|retract <ms>");
        return;
    }
    const uint32_t before_mv = actuator::feedback_mv();
    const uint32_t used_ms = actuator::jog(motion, static_cast<uint32_t>(run_ms));
    Serial.printf("act %s for %lu ms, feedback before %lu mV (use `status` after the move)\n",
                  motion_token, static_cast<unsigned long>(used_ms),
                  static_cast<unsigned long>(before_mv));
    warn_if_disabled();
}

void cmd_imu(char* action_token, char* baud_token) {
    if (action_token != nullptr && strcmp(action_token, "off") == 0) {
        imu_link::stop();
        Serial.println("imu off");
        return;
    }
    long baud = 0;
    if (action_token == nullptr || strcmp(action_token, "on") != 0 ||
        !parse_long(baud_token, baud) || baud <= 0) {
        Serial.println("usage: imu on <baud set on the OpenLog Artemis> | imu off");
        return;
    }
    imu_link::start(static_cast<uint32_t>(baud));
    Serial.printf("imu on at %ld baud, echoing lines\n", baud);
}

void run_command(char* line) {
    char* command = strtok(line, " ");
    if (command == nullptr) {
        return;
    }
    char* arg1 = strtok(nullptr, " ");
    char* arg2 = strtok(nullptr, " ");
    char* arg3 = strtok(nullptr, " ");

    if (strcmp(command, "help") == 0) {
        Serial.print(HELP);
    } else if (strcmp(command, "status") == 0) {
        print_status();
    } else if (strcmp(command, "stream") == 0) {
        stream_ = arg1 != nullptr && strcmp(arg1, "on") == 0;
        if (stream_) {
            print_stream_header();
        }
    } else if (strcmp(command, "en") == 0) {
        const bool on = arg1 != nullptr && strcmp(arg1, "1") == 0;
        tracks::set_driver_enable(on);
        Serial.printf("drivers %s\n", on ? "ON" : "off");
    } else if (strcmp(command, "stop") == 0) {
        tracks::stop();
        actuator::stop();
        tracks::set_driver_enable(false);
        Serial.println("stopped, drivers off (servos keep their pulses, use `servo ... off`)");
    } else if (strcmp(command, "drive") == 0) {
        cmd_drive(arg1, arg2, arg3);
    } else if (strcmp(command, "enc") == 0 && arg1 != nullptr && strcmp(arg1, "reset") == 0) {
        tracks::reset_counts();
        Serial.println("encoder counts reset");
    } else if (strcmp(command, "tof") == 0) {
        Serial.print("tof L ");
        print_tof(Side::LEFT);
        Serial.print(" | R ");
        print_tof(Side::RIGHT);
        Serial.println();
    } else if (strcmp(command, "servo") == 0) {
        cmd_servo(arg1, arg2);
    } else if (strcmp(command, "act") == 0) {
        cmd_act(arg1, arg2);
    } else if (strcmp(command, "btn") == 0) {
        Serial.printf("btn %u raw, %lu mV\n", buttons::raw(),
                      static_cast<unsigned long>(buttons::millivolts()));
    } else if (strcmp(command, "imu") == 0) {
        cmd_imu(arg1, arg2);
    } else if (strcmp(command, "face") == 0) {
        if (arg1 != nullptr && strcmp(arg1, "test") == 0) {
            display::show_test_pattern();
        } else if (arg1 != nullptr) {
            display::show_text(arg1);
        } else {
            Serial.println("usage: face test | face <text>");
        }
    } else {
        Serial.printf("unknown command: %s (type `help`)\n", command);
    }
}

}  // namespace

namespace console {

void begin() {
    Serial.println();
    Serial.println("M.A.T.S firmware, step 1: bench bring-up");
    Serial.println("BENCH ONLY: no edge reflex. Keep the tracks off the ground.");
    Serial.print(HELP);
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
