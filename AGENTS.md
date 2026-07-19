# AGENTS.md — FodorHome Water System

## Project Overview

This repository contains **two Arduino projects** that together form a smart well-water management system for garden irrigation:

| Project | Board | Role |
|---|---|---|
| `Vizrendszer-gh/` | **Arduino Mega 2560** + W5100 Ethernet shield | Main controller: pump control, zone valves, sensors, Blynk app, weather-based scheduling |
| `Vizrendszer-watchdog/` | Small Arduino (Nano/Uno class) | External hardware watchdog: monitors the main unit and resets it if frozen |

The main controller connects to the **Blynk 1.0 (legacy)** IoT platform via a self-hosted server, allowing mobile control and monitoring of the irrigation system.

## Repository Structure

```
├── _libraries/              # Bundled library copies (install into Arduino libraries)
│   ├── Blynk/               # Blynk 1.0 IoT
│   ├── DallasTemperature/   # DS18B20 sensor
│   ├── DHT_sensor_library/  # DHT22 (code commented out, not currently used)
│   ├── OneWire/             # OneWire protocol
│   ├── Time/                # TimeLib
│   └── Utilities/           # Helper macros: LEN(), pinModeGroup(), etc.
│   └── ... (several unused leftovers from removed features)
├── Vizrendszer-gh/          # Main controller (tabbed .ino sketch)
│   ├── Vizrendszer-gh.ino   # Entry point: defines, structs, globals, setup(), loop()
│   ├── blynkEvents.ino      # Blynk virtual pin callbacks & UI sync
│   ├── error.ino            # Error reporting & blocking error handling
│   ├── job.ino              # Top-level job dispatcher (directJobs → water → cool)
│   ├── jobCool.ino           # Cooling stub (removed in v1.13, always returns Continue)
│   ├── jobDo.ino            # Actuator: drives relay outputs from waterJob structs
│   ├── jobRead.ino          # Empty placeholder (future sensor consolidation)
│   ├── jobWatering.ino      # Watering session state machine (unit/weight system)
│   ├── scheduler.ino        # Daily timer + weather-data HTTP fetch + watering decision
│   ├── sense.ino            # Sensor reading: water levels, flow control
│   ├── serialRead.ino       # Empty placeholder
│   ├── serialSend.ino       # Serial debug output (commented out)
│   └── tapFlowControl.ino   # Physical tap switch debounce & toggle
└── Vizrendszer-watchdog/    # External watchdog
    └── vizrendszer-watchdog/
        └── vizrendszer-watchdog.ino
```

## Architecture & Design Patterns

### Tabbed sketch (multi-file .ino)
Arduino IDE concatenates all `.ino` files in the sketch folder alphabetically. Only `Vizrendszer-gh.ino` contains `#include` directives and `#define` constants. All other tabs share globals implicitly — **never add duplicate includes or conflicting definitions** to other tabs.

### Job-based state machine
The core abstraction is `waterJob { stop, from, to }`. The flow is:

```
loop() → sense() → job() → jobDo()
                              ↓
                         Blynk.run()
                         blynkSync() (every 2s)
                         scheduler()
```

- `job()` decides WHAT to do (manual valves, scheduled watering, cooling)
- `jobDo()` executes the current `waterJob` by switching relays without stopping the pump between zone transitions
- `jobStop()` turns everything off

### Weighted proportional zone watering
Each of 6 zones has a weight (set via Blynk). `sumWeights` is the total. During a watering session, `elapsedTime / duration` gives progress, and the active zone is found by iterating through zones subtracting weights from `currentUnit`:

```
zones: [pink(w=5), green(w=3), blue(w=2), ...]
sumWeights = 10, duration = 30 min, elapsed = 15 min
→ currentUnit = (15/30) * 10 = 5
→ pink gets 5, remaining 0 → active zone = green
```

### EEPROM persistence
- Slots 50–56: 7-day watering history (bytes, rolling)
- Watering session data (start time, duration, last-alive) saved for crash recovery
- `dayResetDoneForDay` tracks daily reset to avoid double-counting

### Weather-driven scheduling
Daily at configured time, the system:
1. HTTP GET `irrigation.fodor.pro/lastweek.txt` → mm rainfall last 7 days
2. HTTP GET `irrigation.fodor.pro/today.txt` → mm forecast for today
3. Calculates needed watering minutes via `mmToMinuteFactor`
4. Subtracts already-watered minutes (from EEPROM history)
5. Waters only if remaining > `minimumStartableDuration`

### External hardware watchdog
Separate Arduino monitors a heartbeat on pin A0 (toggled every ~2s by `blynkSync()` on the main unit). If no change for 20s, pulls main unit's RESET pin LOW for 500ms. Has a 50s grace period after reset for the main unit to boot and connect to Blynk + Ethernet.

## Hardware

### Main Controller Pin Map (Mega 2560)

**Outputs (relay board, active LOW):**
| Pin | Function |
|---|---|
| 28 | Main pump |
| 30 | To tap |
| 31 | To grey zone |
| 33 | To pink zone |
| 34 | To green zone |
| 35 | To blue zone |
| 36 | To red zone |
| 8  | Watchdog heartbeat (toggled every 2s) |

**Inputs:**
| Pin | Function |
|---|---|
| A0 | DS18B20 temperature sensor (OneWire, buffer tank) |
| 39, 41, 44, 45, 46, A8 | Water level sensors (upper/lower pairs for buffer & watering tanks) |
| 47 | Physical tap toggle switch (INPUT_PULLUP) |

**Ethernet shield:** W5100 CS = pin 10, SD card CS = pin 4

### Watchdog Pin Map
| Pin | Function |
|---|---|
| A0 | Heartbeat input (from main unit pin 8) |
| 12 | Reset output (to main unit RESET) |
| 13 | Status LED (ON = monitoring, OFF = grace period) |

## Key Conventions

- **`RelayOn = LOW`**, **`RelayOff = HIGH`** — optocoupler relay board is active-low.
- **`Continue / End / Repeat`** — return values for `job()` flow control.
- **`Buffer = 0`**, **`Watering = 1`** — parameters for `levelOf()`.
- **`StopNext / NoStopNext`** — `waterJob.stop` field: whether pump should stop after this transition.
- **`AllValves = 999`** — special sentinel to open all zone valves at once.
- Watering zone indices: `zones[0]=pink, [1]=green, [2]=blue, [3]=red, [4]=grey, [5]=tap`.
- Pin groups are defined as `byte` arrays and controlled with `pinModeGroup()` / `digitalWriteGroup()` from the Utilities library.

## Build & Flash

### Prerequisites
1. Install Arduino IDE (1.8.x or 2.x)
2. Install required libraries from `_libraries/` into your Arduino libraries folder:
   - **Required:** Blynk, DallasTemperature, OneWire, Time, Utilities
   - **Referenced but commented out:** DHT_sensor_library
   - **Not used:** Adafruit_Circuit_Playground, Adafruit_Unified_Sensor, AltSoftSerial, Chronos, HardwareSerial_RS485, RS485_protocol, SD, Servo, SpacebrewYun
3. Create `Vizrendszer-gh/auth.h` (not tracked in git) with:
   ```cpp
   #define AUTH_TOKEN "your-blynk-auth-token"
   #define HOST_ADDRESS "your-blynk-server-ip"
   #define HOST_PORT 8080
   ```

### Build
- **Main controller:** Open `Vizrendszer-gh/Vizrendszer-gh.ino` in Arduino IDE, select **Arduino Mega 2560**, compile & upload.
- **Watchdog:** Open `Vizrendszer-watchdog/vizrendszer-watchdog/vizrendszer-watchdog.ino`, select the appropriate board (Nano/Uno), compile & upload.

### Important
- The `_libraries/` directory contains copies — they must be installed into the Arduino `libraries/` folder (or symlinked) for the IDE to find them.
- `auth.h` is in `.gitignore` — never commit credentials.
- The external PHP weather server is at [github.com/RedyAu/irrigation](https://github.com/RedyAu/irrigation) (separate repository).

## When Making Changes

1. **Don't add duplicate `#include` or `#define`** to tab files — Arduino concatenates them all into one translation unit. Only `Vizrendser-gh.ino` should have headers and defines.
2. **Watch EEPROM layout** — slots 50–56 are the watering history ring buffer. Each slot holds one byte (minutes). Don't overlap.
3. **Relay logic is inverted** — `LOW` = ON, `HIGH` = OFF. `jobStop()` sets all outputs to `RelayOff` (HIGH).
4. **The watchdog heartbeat** on pin 8 must toggle at least every 20 seconds or the watchdog will reset the main unit. `blynkSync()` handles this every 2 seconds.
5. **`initDone` flag** delays Blynk/Ethernet startup by 6 seconds to allow hardware stabilization. Don't move `Blynk.begin()` before this.
6. **Cooling is fully removed** as of v1.13 — `jobCool.ino` is a stub, cooling-related struct fields still exist but are unused. If re-adding cooling, you'll need to re-add temperature sensors, cooling valve control, and the cooling state machine.

## Unused / Leftover Code

Several files and libraries remain from removed features:
- `jobCool.ino` — always returns `Continue`
- `jobRead.ino`, `serialRead.ino` — empty placeholders
- `serialSend.ino` — body is fully commented out
- `DHT_sensor_library` — DHT22 variables declared but code commented out
- `Chronos` — advanced scheduling library, never integrated
- `HardwareSerial_RS485`, `RS485_protocol` — RS485 communication not used
- `SD` — SD card on W5100 shield, CS pin defined but not used
- `Servo`, `SpacebrewYun`, `AltSoftSerial`, `Adafruit_Circuit_Playground` — not referenced

If cleaning up, verify no implicit dependencies before removing libraries.

## Related Repositories

- [irrigation](https://github.com/RedyAu/irrigation) — PHP weather data server that provides rainfall data for the scheduler
- [FodorHome_vizrendszer](https://github.com/RedyAu/FodorHome_vizrendszer) — this repository
