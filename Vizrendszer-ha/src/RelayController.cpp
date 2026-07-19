#include "RelayController.h"

RelayController& RelayController::instance() {
    static RelayController rc;
    return rc;
}

void RelayController::begin() {
    // ── Prevent SD card from interfering with SPI bus ─
    pinMode(config::pins::kSdCardCS, OUTPUT);
    digitalWrite(config::pins::kSdCardCS, HIGH);

    // ── Configure all named relay outputs ────────────
    pinMode(config::pins::kMainPump,  OUTPUT);
    pinMode(config::pins::kTapValve,  OUTPUT);
    pinMode(config::pins::kGreyValve, OUTPUT);
    pinMode(config::pins::kPinkValve, OUTPUT);
    pinMode(config::pins::kGreenValve, OUTPUT);
    pinMode(config::pins::kBlueValve, OUTPUT);
    pinMode(config::pins::kRedValve,  OUTPUT);

    // ── Configure spare relay channels ───────────────
    for (uint8_t i = 0; i < config::pins::kSparePinCount; ++i) {
        pinMode(config::pins::kSparePins[i], OUTPUT);
    }

    // ── Set everything to safe off state ─────────────
    allZonesOff();
    pumpOffInternal();
    for (uint8_t i = 0; i < config::pins::kSparePinCount; ++i) {
        digitalWrite(config::pins::kSparePins[i], config::kRelayOff);
    }
}

void RelayController::update() {
    // Check if grace period for last zone has expired
    if (graceActive_ && millis() >= graceDeadline_) {
        writeRelay(config::kZoneValvePins[graceZone_], false);
        pumpOffInternal();
        graceActive_ = false;
    }
}

// ── Zone Control ────────────────────────────────────

void RelayController::turnOnZone(uint8_t index) {
    if (index >= config::kZoneCount) return;
    if (activeZones_[index]) return;   // already on

    // If this zone was being held open by the grace timer, cancel it
    if (graceActive_ && graceZone_ == index) {
        graceActive_ = false;
        return;   // zone already physically on, just cancel the pending-off
    }

    // Cancel any grace timer (a new zone was added, we're no longer "last off")
    cancelGrace();

    // Turn on the zone relay
    activeZones_[index] = true;
    writeRelay(config::kZoneValvePins[index], true);

    // Ensure pump is running
    if (!pumpOn_) {
        writeRelay(config::pins::kMainPump, true);
        pumpOn_ = true;
    }
}

void RelayController::turnOffZone(uint8_t index) {
    if (index >= config::kZoneCount) return;
    if (!activeZones_[index]) return;   // already off

    activeZones_[index] = false;

    uint8_t remaining = countActiveZones();

    if (remaining > 0) {
        // Other zones still open — turn off this relay immediately, pump stays on
        writeRelay(config::kZoneValvePins[index], false);
    } else {
        // Last zone — start grace timer, keep valve open
        graceActive_   = true;
        graceZone_     = index;
        graceDeadline_ = millis() + config::kPumpOffDelayMs;
        // Valve stays open; pump stays on until timer expires
    }
}

void RelayController::turnOffZoneImmediate(uint8_t index) {
    if (index >= config::kZoneCount) return;
    if (!activeZones_[index]) return;

    activeZones_[index] = false;
    cancelGrace();
    writeRelay(config::kZoneValvePins[index], false);

    if (countActiveZones() == 0) {
        pumpOffInternal();
    }
}

// ── Pump Control ────────────────────────────────────

void RelayController::turnOnPump() {
    if (!pumpOn_) {
        writeRelay(config::pins::kMainPump, true);
        pumpOn_ = true;
    }
}

void RelayController::turnOffPump() {
    // Master kill: turn off ALL zones and pump immediately
    allZonesOff();
    for (uint8_t i = 0; i < config::kZoneCount; ++i) {
        activeZones_[i] = false;
    }
    cancelGrace();
    pumpOffInternal();
}

// ── State Queries ───────────────────────────────────

bool RelayController::isZoneOn(uint8_t index) const {
    if (index >= config::kZoneCount) return false;
    return activeZones_[index];
}

bool RelayController::hasAnyZoneOn() const {
    return countActiveZones() > 0;
}

// ── Internal Helpers ────────────────────────────────

uint8_t RelayController::countActiveZones() const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < config::kZoneCount; ++i) {
        if (activeZones_[i]) ++count;
    }
    return count;
}

void RelayController::cancelGrace() {
    graceActive_ = false;
}

void RelayController::writeRelay(uint8_t pin, bool on) {
    digitalWrite(pin, on ? config::kRelayOn : config::kRelayOff);
}

void RelayController::allZonesOff() {
    for (uint8_t i = 0; i < config::kZoneCount; ++i) {
        digitalWrite(config::kZoneValvePins[i], config::kRelayOff);
    }
}

void RelayController::pumpOffInternal() {
    digitalWrite(config::pins::kMainPump, config::kRelayOff);
    pumpOn_ = false;
}
