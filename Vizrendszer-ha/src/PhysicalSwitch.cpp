#include "PhysicalSwitch.h"
#include "RelayController.h"

bool          PhysicalSwitch::lastStableState_ = false;
bool          PhysicalSwitch::lastReadState_   = false;
unsigned long PhysicalSwitch::lastChangeMs_    = 0;

void PhysicalSwitch::begin() {
    pinMode(config::pins::kTapSwitch, INPUT_PULLUP);
    lastStableState_ = digitalRead(config::pins::kTapSwitch);
    lastReadState_   = lastStableState_;
}

void PhysicalSwitch::update() {
    bool reading = digitalRead(config::pins::kTapSwitch);

    // Detect edge (ignore polarity — we just care about change)
    if (reading != lastReadState_) {
        lastChangeMs_ = millis();
        lastReadState_ = reading;
        return;   // wait for debounce period
    }

    // After 50ms of stable reading, accept the change
    if (reading != lastStableState_ && (millis() - lastChangeMs_ >= 50)) {
        lastStableState_ = reading;

        // Physical switch: HIGH = tap ON, LOW = tap OFF
        // Tap zone index is 5 (last in kZoneValvePins / kZoneNames)
        constexpr uint8_t kTapZoneIndex = 5;

        if (reading == HIGH) {
            RelayController::instance().turnOnZone(kTapZoneIndex);
        } else {
            RelayController::instance().turnOffZoneImmediate(kTapZoneIndex);
        }
    }
}
