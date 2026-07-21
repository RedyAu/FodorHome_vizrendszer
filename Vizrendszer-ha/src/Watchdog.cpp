#include "Watchdog.h"
#include "HaBridge.h"

unsigned long Watchdog::lastToggleMs_  = 0;
bool          Watchdog::state_         = false;

void Watchdog::begin() {
    pinMode(config::pins::kWatchdog, OUTPUT);
    digitalWrite(config::pins::kWatchdog, LOW);
    lastToggleMs_  = millis();
}

void Watchdog::update() {
    unsigned long now = millis();

    // Startup grace: first kStartupGraceMs always allow heartbeat
    // (DHCP + MQTT connect need time). After that, only toggle if
    // MQTT is actually connected — checked directly, no async state.
    bool startupGrace = (now - lastToggleMs_ < config::kStartupGraceMs);
    bool mqttOk       = HaBridge::instance().isConnected();

    if (!startupGrace && !mqttOk) {
        return;   // keep pin LOW → watchdog will reset us
    }

    if (now - lastToggleMs_ >= config::kWatchdogIntervalMs) {
        state_ = !state_;
        digitalWrite(config::pins::kWatchdog, state_ ? HIGH : LOW);
        lastToggleMs_ = now;
    }
}
