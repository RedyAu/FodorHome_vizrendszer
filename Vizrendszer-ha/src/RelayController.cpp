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
    // Check if a pending turn-off grace period has expired
    if (pendingOffZoneIndex_ != -1 && millis() >= pendingOffDeadline_) {
        writeRelay(config::kZoneValvePins[pendingOffZoneIndex_], false);
        pumpOffInternal();
        activeZoneIndex_ = -1;
        pendingOffZoneIndex_ = -1;
    }
}

// ── Zone Control ────────────────────────────────────

void RelayController::turnOnZone(uint8_t index) {
    if (index >= config::kZoneCount) return;

    // If this zone is already pending-off, cancel the timer (zone staying on)
    if (pendingOffZoneIndex_ == index) {
        pendingOffZoneIndex_ = -1;
        return;
    }

    // If a DIFFERENT zone is active, turn it off immediately (seamless switch)
    if (activeZoneIndex_ != -1 && activeZoneIndex_ != index) {
        writeRelay(config::kZoneValvePins[activeZoneIndex_], false);
    }

    // Cancel any pending turn-off (different zone or no-longer-relevant)
    pendingOffZoneIndex_ = -1;

    // Turn on the requested zone
    writeRelay(config::kZoneValvePins[index], true);
    activeZoneIndex_ = index;

    // Ensure pump is running
    if (!pumpOn_) {
        writeRelay(config::pins::kMainPump, true);
        pumpOn_ = true;
    }
}

void RelayController::turnOffZone(uint8_t index) {
    if (index >= config::kZoneCount) return;

    // Only act if this is the active zone
    if (activeZoneIndex_ != index) return;

    // Start the 1-second grace timer — don't turn anything off yet
    pendingOffZoneIndex_ = index;
    pendingOffDeadline_  = millis() + config::kPumpOffDelayMs;
}

void RelayController::turnOffZoneImmediate(uint8_t index) {
    if (index >= config::kZoneCount) return;
    if (activeZoneIndex_ != index) return;

    // Cancel any pending grace timer
    pendingOffZoneIndex_ = -1;

    // Turn off zone and pump immediately
    writeRelay(config::kZoneValvePins[index], false);
    pumpOffInternal();
    activeZoneIndex_ = -1;
}

// ── Pump Control ────────────────────────────────────

void RelayController::turnOnPump() {
    if (!pumpOn_) {
        writeRelay(config::pins::kMainPump, true);
        pumpOn_ = true;
    }
}

void RelayController::turnOffPump() {
    // Master kill: turn off pump AND any active zone immediately
    if (activeZoneIndex_ != -1) {
        writeRelay(config::kZoneValvePins[activeZoneIndex_], false);
        activeZoneIndex_ = -1;
    }
    pendingOffZoneIndex_ = -1;   // cancel any grace timer
    pumpOffInternal();
}

// ── State Queries ───────────────────────────────────

bool RelayController::isZoneOn(uint8_t index) const {
    if (index >= config::kZoneCount) return false;
    return activeZoneIndex_ == index;
}

// ── Internal Helpers ────────────────────────────────

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
