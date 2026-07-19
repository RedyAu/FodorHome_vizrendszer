#pragma once

#include <Arduino.h>
#include "config.h"

/// Singleton that manages all relay outputs with zone-switching safety logic.
///
/// Key safety behaviours:
/// - Multiple zones can be active simultaneously.
/// - Pump is ON whenever any zone is ON.
/// - 1-second grace period on the LAST zone turn-off: allows HA command-chaining
///   without pump cycling. During the grace period the last zone valve stays open.
/// - Pump OFF command acts as a master kill: stops pump + all zones.
class RelayController {
public:
    static RelayController& instance();

    /// Initialize all relay pins to safe (off) state.
    void begin();

    /// Called every loop(). Checks and expires the grace-period timer.
    void update();

    // ── Zone Control ────────────────────────────────

    /// Turn on a zone by index (0..5). Auto-starts pump.
    /// Does NOT affect other already-active zones.
    void turnOnZone(uint8_t index);

    /// Turn off a zone by index. If other zones remain active, the relay
    /// turns off immediately. If this is the LAST zone, a 1-second grace
    /// timer starts before the valve and pump turn off.
    void turnOffZone(uint8_t index);

    /// Turn off a zone immediately — no grace period, even for the last zone.
    /// Used by the physical tap switch for instant response.
    void turnOffZoneImmediate(uint8_t index);

    // ── Pump Control ────────────────────────────────

    /// Turn on the pump relay directly (for manual test).
    /// Rejected if no zone is active (pump must never run dry).
    void turnOnPump();

    /// Master kill: turn off pump AND all zones immediately.
    void turnOffPump();

    // ── State Queries ───────────────────────────────

    bool isZoneOn(uint8_t index) const;
    bool isPumpOn()  const { return pumpOn_; }

    /// Returns true if at least one zone valve is open.
    bool hasAnyZoneOn() const;

private:
    RelayController() = default;

    void writeRelay(uint8_t pin, bool on);
    void allZonesOff();
    void pumpOffInternal();
    uint8_t countActiveZones() const;
    void cancelGrace();

    // State
    bool activeZones_[config::kZoneCount] = {false};
    bool pumpOn_ = false;

    // Grace period (last-zone-off delay)
    bool          graceActive_   = false;
    uint8_t       graceZone_     = 0;     // zone held open during grace
    unsigned long graceDeadline_ = 0;     // millis() deadline
};
