# DEPRECATED — Vizrendszer-gh (Blynk-based firmware)

This folder contains the **old Arduino Mega 2560 firmware** (v1.13.1-rc2) that
uses the Blynk 1.0 IoT platform for irrigation control.

**It has been replaced by [`Vizrendszer-ha/`](../Vizrendszer-ha/)** — a stripped-down
firmware that exposes the pump and zone valve relays as individual switches in
Home Assistant via MQTT. All irrigation logic is now implemented as HA automations.

## Why the change?

- Blynk 1.0 is legacy and unsafe to self-host.
- Deploying changes required physical Arduino IDE uploads — slow and error-prone.
- With HA handling the scheduling, weather logic, and zone sequencing,
  the Arduino only needs to be a reliable relay bridge.

## What moved where?

| Old (Blynk .ino) | New (HA) |
|---|---|
| Daily scheduler | HA time-based automation |
| Weather HTTP fetch | HA RESTful sensors |
| Weighted zone watering | HA script with `switch.*` calls |
| Manual zone toggles | HA dashboard switches |
| EEPROM history ring buffer | HA `history_stats` / input helpers |
| Error handling | HA notifications |

## The irrigation algorithm

The full original irrigation logic has been documented in
[`irrigation-ha-logic.md`](../irrigation-ha-logic.md) for reimplementation
in Home Assistant.

## The hardware watchdog

[`Vizrendszer-watchdog/`](../Vizrendszer-watchdog/) is **unchanged** and still
required — it monitors the main unit's heartbeat and resets it if frozen.

## Build instructions (old, for reference)

1. Open `Vizrendszer-gh.ino` in Arduino IDE.
2. Select **Arduino Mega 2560**.
3. Create `auth.h` with Blynk credentials (see original AGENTS.md).
4. Compile & upload.
