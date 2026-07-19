# Irrigation Logic — For Home Assistant Reimplementation

Extracted from `Vizrendszer-gh` v1.13.1-rc2 (Arduino Mega 2560 firmware).  
This document preserves the full algorithm to be reimplemented as a Home Assistant automation/script.

---

## 1. Weather-Driven Watering Calculation

### Data Sources
Two HTTP endpoints served by a PHP server ([RedyAu/irrigation](https://github.com/RedyAu/irrigation)):
- `GET irrigation.fodor.pro/lastweek.txt` → rainfall in mm over the last 7 days, value wrapped in `{value}` (e.g., `{12.5}`)
- `GET irrigation.fodor.pro/today.txt` → forecast rainfall in mm for today, same format

### Calculation
```
sumMinutesNeeded  = (mmForWeek + mmForToday) × mmToMinuteFactor
calculatedMinutes = sumMinutesNeeded - sumMinutesWateredOnDays()
```
- `mmToMinuteFactor`: user-configurable constant (units: **minutes** of watering per mm of rainfall deficit). Default ~300.
- `sumMinutesWateredOnDays()`: sum of the last 6 days' watering minutes from the history ring buffer (excludes today).
- Watering triggers only if `calculatedMinutes > minimumStartableDuration` (configurable, typically 120 seconds = 2 minutes).
- Note: despite the variable name, `mmToMinuteFactor` produces **minutes** directly (not seconds). The result is multiplied by 60 before being used as a session duration in seconds.

### Variables to Expose in HA
| HA Helper | Type | Purpose |
|-----------|------|---------|
| `input_number.mm_to_minute_factor` | number | mm rainfall deficit → watering minutes |
| `input_number.minimum_startable_duration` | number | Minimum seconds to trigger auto-watering |
| `sensor.rainfall_last_7_days` | RESTful sensor | mm rainfall last week from irrigation.fodor.pro |
| `sensor.rainfall_forecast_today` | RESTful sensor | mm forecast today from irrigation.fodor.pro |
| `sensor.calculated_watering_minutes` | template sensor | Computed needed minutes (display only) |

---

## 2. Daily Scheduling

### Original Logic
- Checked every `loop()`: when `secondsSinceMidnight > dailyWateringAtSeconds` AND `!doneToday` → trigger.
- `dailyWateringAtSeconds`: seconds-from-midnight, user-configurable.
- `doneToday`: flag set after today's watering completes (or manually).

### Flow
1. Fetch weather data (up to 5 retries on failure).
2. Calculate `calculatedDuration` (see §1).
3. If `calculatedDuration > minimumStartableDuration`: start watering session.
4. Otherwise: log skip.

### HA Implementation
- **Trigger**: time-based automation at the configured time of day.
- **Condition**: `input_boolean.done_today` is off AND `input_boolean.periodic_watering_enabled` is on.
- **Actions**: call RESTful sensors → compute → conditionally start watering script.

### HA Helpers
| Helper | Type | Purpose |
|--------|------|---------|
| `input_datetime.daily_watering_time` | time | When to run daily watering |
| `input_boolean.periodic_watering_enabled` | toggle | Enable/disable the daily scheduler |
| `input_boolean.skip_next_watering` | toggle | Skip the next scheduled run |
| `input_boolean.done_today` | toggle | Today's watering completed (or manual override) |

---

## 3. Weighted Proportional Zone Watering

### Zone Configuration
Six zones, each with:
- `isActive` (bool) — whether the zone participates in auto-watering
- `weight` (int) — proportional weight for time distribution

Zone order (fixed): **Pink → Green → Blue → Red → Grey → Tap**

### Algorithm
```
sumWeights = Σ(active_zone.weight)

At any time t during session:
  ratio       = elapsedTime / totalDuration
  currentUnit = sumWeights × ratio        (integer truncated)
```

To find the active zone at `currentUnit`:
```
temp = currentUnit
for each zone in order (Pink, Green, Blue, Red, Grey, Tap):
    if zone.isActive:
        temp -= zone.weight
        if temp < 0:
            active zone = this zone
            break
```

### Example
Zones: Pink(w=5), Green(w=3), Blue(w=2). sumWeights=10. Duration=30min.

| Elapsed | Ratio | currentUnit | Active Zone |
|---------|-------|-------------|-------------|
| 0–15 min | 0.0–0.5 | 0–5 | Pink |
| 15–24 min | 0.5–0.8 | 5–8 | Green |
| 24–30 min | 0.8–1.0 | 8–10 | Blue |

### Zone Snapshot
When a watering session starts, the current zone configuration is **snapshotted** and used throughout the session. Changes made mid-session (e.g., toggling a zone off) do NOT affect the running session.

### HA Implementation
The watering script would:
1. Read zone weights and active flags from HA helpers.
2. Calculate per-zone durations: `zoneDuration[i] = totalDuration × (weight[i] / sumWeights)`.
3. Sequence through zones with `switch.turn_on` / `switch.turn_off` calls and `delay` templates.
4. The Arduino firmware handles seamless pump continuity (1-second grace period on zone switching).

### HA Helpers (per zone)
| Active Helper | Weight Helper | Zone |
|---------------|---------------|------|
| `input_boolean.zone_pink_active` | `input_number.zone_pink_weight` | Pink |
| `input_boolean.zone_green_active` | `input_number.zone_green_weight` | Green |
| `input_boolean.zone_blue_active` | `input_number.zone_blue_weight` | Blue |
| `input_boolean.zone_red_active` | `input_number.zone_red_weight` | Red |
| `input_boolean.zone_grey_active` | `input_number.zone_grey_weight` | Grey |
| `input_boolean.zone_tap_active` | `input_number.zone_tap_weight` | Tap |

---

## 4. Session Management & Dead-Time Compensation

### Original Logic
- `beginWatering(duration)`: records start time, snapshots zones, saves to EEPROM for crash recovery.
- `water()`: called every loop during active session.
  - If no update for >10s AND `canMoveStart` is true: shift `startTime` forward by the idle duration (compensates for pump-off time, e.g., low water level).
  - Update `lastAlive`, save to EEPROM every 10s.
  - Calculate `currentUnit` and select active zone.
  - When `currentUnit >= sumWeights`: session complete. Add `duration/60` minutes to daily total.

### Dead-Time Compensation
When the pump stops (e.g., low water level), the progress clock pauses:
```
deadSince = now() - lastAlive
if deadSince > 10 seconds:
    startTime = now() - (lastAlive - startTime)    # shift start forward
```
This effectively pauses the watering timer. When water is available again, the session continues where it left off.

### HA Implementation
HA handles this differently — the script/automation simply won't advance while waiting. A simpler approach:
- Track `elapsed_watering_seconds` in an `input_number` helper.
- The script increments it periodically while the pump is confirmed running.
- Or: use wall-clock time and an `input_datetime.watering_paused_at` to track pause durations.

### Session Persistence
The original used EEPROM (slots 13–26) to survive power loss. In HA:
- Active session state lives in HA helpers (survives HA restarts).
- If the Arduino loses power, HA will detect it offline (MQTT LWT). The automation can pause and resume when the device comes back.

---

## 5. Daily History & Ring Buffer

### Original EEPROM Layout
| Slots | Size | Content |
|-------|------|---------|
| 13 | 1 byte | `wateringFinished` flag |
| 14 | 1 byte | Session purpose |
| 15–18 | 4 bytes | `startTime` (unsigned long) |
| 19–22 | 4 bytes | `lastAlive` (unsigned long) |
| 23–26 | 4 bytes | `duration` (unsigned long) |
| 50–56 | 7 bytes | Watering history ring buffer (1 byte = minutes per day) |

### Daily Reset (at midnight, detected by day-of-month change)
1. Shift ring buffer: `slot[i+1] = slot[i]` for i = 0..5.
2. Write `wateringMinutesCompletedToday` to slot 0 (most recent day).
3. Reset `wateringMinutesCompletedToday = 0`.
4. Set `doneToday = false`.
5. `sumMinutesWateredOnDays()` sums slots 0–5 (last 6 days, excluding slot 6 = current day).

### HA Implementation
Replace the EEPROM ring buffer with:
- `input_number.watering_minutes_today` — resets at midnight via automation.
- 7 `input_number` helpers for history, shifted daily via automation.
- Or use HA's built-in history statistics (`history_stats` sensor) to sum `switch` on-time over the past N days.

---

## 6. Manual Controls (from Blynk → HA)

| Original Blynk | New HA Equivalent | Behavior |
|---------------|-------------------|----------|
| V40–V44, V50 (direct zone toggles) | `switch.zone_*` entities | Toggle individual zones on/off |
| V49 (all valves open) | Script or separate switch | Open all 6 zone valves simultaneously |
| V52 (emergency stop) | `switch.pump` turn_off | Stops pump + all zones |
| V56 (manual watering start/stop) | Script or `input_button` | Start/stop a manual watering session |
| V57 (manual watering duration) | `input_number` | Duration in minutes for manual watering |
| V80 (pause) | `input_boolean.pause_watering` | Global pause |

---

## 7. Zone Pin Map (Arduino Mega 2560 — unchanged)

| Zone | Arduino Pin | Relay |
|------|------------|-------|
| Main Pump | 28 | Controls well pump contactor |
| Tap | 30 | Garden tap / hose spigot |
| Grey (Dump) | 31 | Grey zone / tank dump valve |
| Pink | 33 | Pink zone solenoid |
| Green | 34 | Green zone solenoid |
| Blue | 35 | Blue zone solenoid |
| Red | 36 | Red zone solenoid |

**All relays are active-LOW**: `ON = LOW (0V)`, `OFF = HIGH (5V)`.

---

## 8. Key Constants Carried Forward

| Constant | Value | Meaning |
|----------|-------|---------|
| `recalculateInactiveFor` | 10 seconds | Dead-time threshold before shifting startTime |
| `minimumStartableDuration` | configurable (~120s) | Minimum seconds to trigger auto-watering |
| `mmToMinuteFactor` | configurable (~300) | mm rainfall deficit → watering minutes |
| `RelayOn` | LOW (0) | Active-low optocoupler relay |
| `RelayOff` | HIGH (1) | Relay inactive |

---

## 9. HA Automation Outline (Pseudo-YAML)

```yaml
# Daily scheduler
automation:
  - alias: "Irrigation — Daily Watering"
    trigger:
      - platform: time
        at: input_datetime.daily_watering_time
    condition:
      - condition: state
        entity_id: input_boolean.periodic_watering_enabled
        state: "on"
      - condition: state
        entity_id: input_boolean.done_today
        state: "off"
      - condition: state
        entity_id: input_boolean.skip_next_watering
        state: "off"
    action:
      # 1. Fetch weather
      - service: homeassistant.update_entity
        target:
          entity_id:
            - sensor.rainfall_last_7_days
            - sensor.rainfall_forecast_today
      # 2. Calculate needed minutes (template sensor does this)
      # 3. Condition: if calculated_minutes > minimum_startable_duration
      # 4. Call watering script with calculated duration
      - service: script.turn_on
        target:
          entity_id: script.irrigation_watering_cycle
        data:
          variables:
            duration_seconds: "{{ states('sensor.calculated_watering_seconds') | int }}"
      # 5. Mark done
      - service: input_boolean.turn_on
        target:
          entity_id: input_boolean.done_today

# Watering cycle script
script:
  irrigation_watering_cycle:
    fields:
      duration_seconds:
        name: "Total Duration"
        description: "Total watering duration in seconds"
    sequence:
      # 1. Read zone config, calculate per-zone durations
      # 2. For each active zone in order:
      #    - switch.turn_on zone_*
      #    - wait for calculated zone duration
      #    - switch.turn_off zone_*
      # 3. Update history
      # 4. Reset done_today if manual

# Midnight reset
automation:
  - alias: "Irrigation — Daily Reset"
    trigger:
      - platform: time
        at: "00:00:00"
    action:
      # Shift history ring buffer
      # Reset watering_minutes_today = 0
      # Set done_today = false
      # Set skip_next_watering = false
```

---

## 10. Related Repositories

- [RedyAu/irrigation](https://github.com/RedyAu/irrigation) — PHP weather data server
- [RedyAu/FodorHome_vizrendszer](https://github.com/RedyAu/FodorHome_vizrendszer) — this repository (Arduino firmware)
- [dawidchyrzynski/arduino-home-assistant](https://github.com/dawidchyrzynski/arduino-home-assistant) — MQTT/HA library used by the new firmware
