/// Vizrendszer-ha — Arduino Mega 2560 Relay Bridge for Home Assistant
///
/// Stripped-down firmware that exposes 7 relay switches (pump + 6 zone valves)
/// to Home Assistant via MQTT auto-discovery.  All irrigation logic is moved
/// to HA automations — see irrigation-ha-logic.md.
///
/// Hardware: Arduino Mega 2560 + W5100 Ethernet Shield + 8-channel relay board
/// External watchdog: separate Arduino monitoring heartbeat on pin 8

#include <Arduino.h>
#include "config.h"
#include "RelayController.h"
#include "HaBridge.h"
#include "Watchdog.h"
#include "PhysicalSwitch.h"

void setup() {
    Serial.begin(115200);
    Serial.print(F("\n── Vizrendszer-ha "));
    Serial.print(config::kSoftwareVersion);
    Serial.println(F(" ──"));

    // ── 1. Initialize hardware ──────────────────────
    RelayController::instance().begin();
    Watchdog::begin();
    PhysicalSwitch::begin();

    // ── 2. Initialize HA bridge (Ethernet + MQTT) ───
    HaBridge::instance().begin();

    Serial.println(F("[main] Running."));
}

void loop() {
    Watchdog::update();                    // keep the watchdog happy
    PhysicalSwitch::update();              // physical tap switch
    HaBridge::instance().update();         // MQTT + state sync
    RelayController::instance().update();  // grace period timer
}
