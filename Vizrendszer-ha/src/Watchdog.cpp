#include "Watchdog.h"

unsigned long Watchdog::lastToggleMs_  = 0;
unsigned long Watchdog::lastHealthyMs_ = 0;
bool          Watchdog::state_         = false;

void Watchdog::begin() {
    pinMode(config::pins::kWatchdog, OUTPUT);
    digitalWrite(config::pins::kWatchdog, LOW);
    lastToggleMs_  = millis();
    lastHealthyMs_ = millis();   // startup grace: assume healthy initially
}

void Watchdog::update() {
    unsigned long now = millis();

    // Startup grace: first kStartupGraceMs always allow heartbeat
    // (DHCP + MQTT connect need time). After that, require recent
    // health confirmation within kWatchdogHealthWindowMs.
    bool startupGrace = (now - lastToggleMs_ < config::kStartupGraceMs);
    bool mqttOk       = (now - lastHealthyMs_ < config::kWatchdogHealthWindowMs);

    if (!startupGrace && !mqttOk) {
        return;   // keep pin LOW → watchdog will reset us
    }

    if (now - lastToggleMs_ >= config::kWatchdogIntervalMs) {
        state_ = !state_;
        digitalWrite(config::pins::kWatchdog, state_ ? HIGH : LOW);
        lastToggleMs_ = now;
    }
}

void Watchdog::setHealthy(bool ok) {
    if (ok) {
        lastHealthyMs_ = millis();
    }
}
