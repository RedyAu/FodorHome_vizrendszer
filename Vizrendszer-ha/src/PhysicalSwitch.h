#pragma once

#include <Arduino.h>
#include "config.h"

/// Monitors the physical toggle switch at pin 47 (INPUT_PULLUP).
/// When toggled, directly controls the tap zone valve via RelayController.
/// State changes are picked up by HaBridge's normal sync cycle and
/// reported back to Home Assistant.
class PhysicalSwitch {
public:
    /// Configure pin as INPUT_PULLUP, read initial state.
    static void begin();

    /// Called every loop(). Reads pin, debounces, and toggles tap zone
    /// on state change.
    static void update();

private:
    static bool          lastStableState_;
    static bool          lastReadState_;
    static unsigned long lastChangeMs_;
};
