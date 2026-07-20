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
/// The heartbeat only toggles if the MQTT connection is confirmed healthy
/// via a round-trip ping (see setHealthy()).  If MQTT becomes unresponsive,
/// the heartbeat stops, forcing a hardware reset.
class Watchdog {
public:
    /// Set the heartbeat pin as OUTPUT.
    static void begin();

    /// Called every loop(). Toggles the heartbeat pin every kWatchdogIntervalMs,
    /// but ONLY if the MQTT connection has been confirmed healthy.
    static void update();

    /// Call this periodically to report whether MQTT is alive.
    /// If called with `false` for too long, the heartbeat stops
    /// and the external watchdog will reset the system.
    static void setHealthy(bool ok);

private:
    static unsigned long lastToggleMs_;
    static unsigned long lastHealthyMs_;   // last time setHealthy(true) was called
    static bool          state_;
};
