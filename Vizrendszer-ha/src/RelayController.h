#pragma once

#include <Arduino.h>
#include "config.h"

/// Singleton that manages all relay outputs with zone-switching safety logic.
///
/// Key safety behaviours:
/// - Only one zone valve active at a time.
/// - Switching zones does NOT stop the pump (seamless transition).
/// - 1-second grace period on all turn-offs: allows HA command-chaining
///   without pump cycling.
/// - Pump OFF command acts as a master kill: stops pump + all zones.
class RelayController {
public:
    static RelayController& instance();

    /// Initialize all relay pins to safe (off) state.
    void begin();

    /// Called every loop(). Checks and expires pending turn-off timers.
    void update();

    // ── Zone Control ────────────────────────────────

    /// Turn on a zone by index (0..5). Auto-starts pump.
    /// If a different zone is active, it's turned off immediately (seamless switch).
    void turnOnZone(uint8_t index);

    /// Turn off a zone by index. Starts 1-second grace timer.
    /// The actual relay turns off only after the grace period expires
    /// (or immediately if the pump is turned off via turnOffPump).
    void turnOffZone(uint8_t index);

    /// Turn off a zone immediately — no grace period.
    /// Used by the physical tap switch for instant response.
    void turnOffZoneImmediate(uint8_t index);

    // ── Pump Control ────────────────────────────────

    /// Turn on the pump relay directly (for manual test).
    void turnOnPump();

    /// Master kill: turn off pump AND all zones immediately.
    void turnOffPump();

    // ── State Queries ───────────────────────────────

    bool isZoneOn(uint8_t index) const;
    bool isPumpOn()  const { return pumpOn_; }

    /// Returns the currently active zone index, or -1 if none.
    int8_t activeZone() const { return activeZoneIndex_; }

private:
    RelayController() = default;

    void writeRelay(uint8_t pin, bool on);
    void allZonesOff();
    void pumpOffInternal();

    // State
    int8_t  activeZoneIndex_   = -1;       // 0..5 or -1 = none
    bool    pumpOn_            = false;

    // Pending turn-off (grace period)
    int8_t        pendingOffZoneIndex_ = -1;   // -1 = none pending
    unsigned long pendingOffDeadline_  = 0;    // millis() deadline
};
