#pragma once

#include <Arduino.h>

// ── MQTT Broker secrets (not tracked in git) ────────
//    Copy secrets.h.example → secrets.h and fill in your values.
#include "secrets.h"

// ────────────────────────────────────────────────────
// Vizrendszer-ha — Configuration
// Arduino Mega 2560 relay bridge for Home Assistant
// ────────────────────────────────────────────────────

namespace config {

// ── Software Identity ───────────────────────────────
constexpr const char* kSoftwareVersion = "2.0.0";
constexpr const char* kDeviceName      = "FodorHome Water System";
constexpr const char* kManufacturer    = "RedyAu";

// ── Relay Logic (active-LOW optocoupler board) ──────
constexpr bool kRelayOn  = LOW;
constexpr bool kRelayOff = HIGH;

// ── Zone Valve Pin Map ──────────────────────────────
namespace pins {
    constexpr uint8_t kMainPump  = 28;
    constexpr uint8_t kTapValve  = 30;
    constexpr uint8_t kGreyValve = 31;   // aka Dump
    constexpr uint8_t kPinkValve = 33;
    constexpr uint8_t kGreenValve = 34;
    constexpr uint8_t kBlueValve = 35;
    constexpr uint8_t kRedValve  = 36;
    constexpr uint8_t kWatchdog  = 8;

    // Inputs
    constexpr uint8_t kTapSwitch = 47;   // physical toggle (INPUT_PULLUP)

    // Ethernet shield
    constexpr uint8_t kW5100_CS  = 10;
    constexpr uint8_t kSdCardCS  = 4;

    // Spare relay channels (initialized to off for safety)
    constexpr uint8_t kSparePins[] = {22, 23, 24, 25, 26, 27, 29, 32, 37};
    constexpr uint8_t kSparePinCount = sizeof(kSparePins) / sizeof(kSparePins[0]);
} // namespace pins

// ── Zone Definitions ────────────────────────────────
constexpr uint8_t kZoneCount = 6;

// Zone valve pins in display/priority order: Pink, Green, Blue, Red, Grey, Tap
constexpr uint8_t kZoneValvePins[kZoneCount] = {
    pins::kPinkValve,
    pins::kGreenValve,
    pins::kBlueValve,
    pins::kRedValve,
    pins::kGreyValve,
    pins::kTapValve,
};

// HA entity unique IDs (must match ArduinoHA HASwitch constructor)
constexpr const char* kZoneUniqueIds[kZoneCount] = {
    "zonePink",
    "zoneGreen",
    "zoneBlue",
    "zoneRed",
    "zoneGrey",
    "zoneTap",
};

// HA friendly names
constexpr const char* kZoneNames[kZoneCount] = {
    "Ház előtt zóna",
    "Garázs mellett zóna",
    "Nagy udvar zóna",
    "Árnyékos udvar zóna",
    "Utca zóna",
    "Kerti csap",
};

// ── Pump ───────────────────────────────────────────
constexpr const char* kPumpUniqueId = "pump";
constexpr const char* kPumpName     = "Szivattyú";

// ── Timing (milliseconds) ──────────────────────────
constexpr unsigned long kWatchdogIntervalMs     = 2000;    // heartbeat toggle rate
constexpr unsigned long kPumpOffDelayMs         = 1000;    // grace period before turning off pump
constexpr unsigned long kStartupGraceMs         = 30000;   // heartbeat always on during first 30s

// ── MAC Address (from existing W5100 shield) ───────
//    If your shield has a different MAC, change it here.
constexpr uint8_t kMacAddress[6] = {
    0x06, 0x39, 0xD4, 0x60, 0xB5, 0x96
};

} // namespace config
