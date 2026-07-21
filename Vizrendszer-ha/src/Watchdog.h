#pragma once

#include <Arduino.h>
#include "config.h"

/// Manages the heartbeat signal sent to the external hardware watchdog.
///
/// The watchdog is a separate Arduino (Nano/Uno) monitoring pin A0
/// connected to the main unit's pin 8 (kWatchdog).  If the heartbeat
/// stops toggling for ~20 seconds, the watchdog pulls the main unit's
/// RESET pin LOW for 500ms, performing a hardware reset.
///
/// The heartbeat toggles every kWatchdogIntervalMs only while the MQTT
/// connection is alive.  Connectivity is checked directly (inline) before
/// each toggle — no async health flag, no ping round-trip.  If MQTT drops,
/// the heartbeat stops immediately, forcing a hardware reset.
class Watchdog {
public:
    /// Set the heartbeat pin as OUTPUT.
    static void begin();

    /// Called every loop(). Toggles the heartbeat pin every kWatchdogIntervalMs,
    /// but ONLY if the MQTT connection is currently alive (checked inline).
    /// During the first kStartupGraceMs the heartbeat always toggles to give
    /// DHCP + MQTT time to connect.
    static void update();

private:
    static unsigned long lastToggleMs_;
    static bool          state_;
};
