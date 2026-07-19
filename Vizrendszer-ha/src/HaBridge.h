#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoHA.h>
#include "config.h"

/// Manages the MQTT connection to Home Assistant and exposes
/// 7 switches (pump + 6 zone valves) via ArduinoHA auto-discovery.
class HaBridge {
public:
    static HaBridge& instance();

    /// Initialize Ethernet (DHCP), configure HA device & switches, begin MQTT.
    void begin();

    /// Called every loop(). Services Ethernet, MQTT, and syncs relay state to HA.
    void update();

private:
    HaBridge() = default;

    // ── Callbacks ───────────────────────────────────
    static void onZoneCommand(bool state, HASwitch* sender);
    static void onPumpCommand(bool state, HASwitch* sender);

    // ── State sync ──────────────────────────────────
    void syncStates();

    // ── Ethernet / MQTT ─────────────────────────────
    EthernetClient ethClient_;
    HADevice        device_{config::kMacAddress, sizeof(config::kMacAddress)};
    HAMqtt          mqtt_{ethClient_, device_, 15};  // up to 15 device types

    // ── HA Entities ─────────────────────────────────
    HASwitch pumpSwitch_{config::kPumpUniqueId};
    HASwitch zoneSwitches_[config::kZoneCount] = {
        HASwitch(config::kZoneUniqueIds[0]),
        HASwitch(config::kZoneUniqueIds[1]),
        HASwitch(config::kZoneUniqueIds[2]),
        HASwitch(config::kZoneUniqueIds[3]),
        HASwitch(config::kZoneUniqueIds[4]),
        HASwitch(config::kZoneUniqueIds[5]),
    };

    // ── Last known state (for change detection) ─────
    bool lastPumpState_ = false;
    bool lastZoneStates_[config::kZoneCount] = {false};
};
