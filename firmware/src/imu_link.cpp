#include "imu_link.h"

#include "config.h"
#include "pins.h"

namespace {

bool running_ = false;
bool echo_ = false;
char line_[config::IMU_LINE_MAX + 1] = {};
size_t length_ = 0;
uint32_t line_count_ = 0;
uint32_t last_line_ms_ = 0;

}  // namespace

namespace imu_link {

void start(uint32_t baud) {
    // With a TX pin of -1 on the first start, the core attaches no TX pin, so D8 (the default
    // UART1 TX pin and RIGHT_IN1) is left alone (HardwareSerial.cpp, begin()).
    Serial1.begin(baud, SERIAL_8N1, pins::IMU_RX, -1);
    running_ = true;
    echo_ = true;
    length_ = 0;
}

void stop() {
    Serial1.end();
    running_ = false;
    echo_ = false;
}

bool running() {
    return running_;
}

void set_echo(bool on) {
    echo_ = on;
}

void update(Print& out, uint32_t now_ms) {
    if (!running_) {
        return;
    }
    while (Serial1.available() > 0) {
        const char c = static_cast<char>(Serial1.read());
        if (c == '\r') {
            continue;
        }
        if (c == '\n' || length_ == config::IMU_LINE_MAX) {
            line_[length_] = '\0';
            if (length_ > 0) {
                ++line_count_;
                last_line_ms_ = now_ms;
                if (echo_) {
                    out.print("imu: ");
                    out.println(line_);
                }
            }
            length_ = 0;
            if (c == '\n') {
                continue;
            }
        }
        line_[length_++] = c;
    }
}

uint32_t line_count() {
    return line_count_;
}

uint32_t last_line_ms() {
    return last_line_ms_;
}

}  // namespace imu_link
