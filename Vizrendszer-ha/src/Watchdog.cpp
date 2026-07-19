#include "Watchdog.h"

unsigned long Watchdog::lastToggleMs_ = 0;
bool          Watchdog::state_        = false;

void Watchdog::begin() {
    pinMode(config::pins::kWatchdog, OUTPUT);
    digitalWrite(config::pins::kWatchdog, LOW);
    lastToggleMs_ = millis();
}

void Watchdog::update() {
    unsigned long now = millis();
    if (now - lastToggleMs_ >= config::kWatchdogIntervalMs) {
        state_ = !state_;
        digitalWrite(config::pins::kWatchdog, state_ ? HIGH : LOW);
        lastToggleMs_ = now;
    }
}
