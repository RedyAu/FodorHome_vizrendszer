#include "HaBridge.h"
#include "RelayController.h"
#include "Watchdog.h"

HaBridge& HaBridge::instance() {
    static HaBridge bridge;
    return bridge;
}

void HaBridge::begin() {
    Serial.println(F("[HaBridge] Starting Ethernet with DHCP..."));
    if (Ethernet.begin(const_cast<uint8_t*>(config::kMacAddress)) == 0) {
        Serial.println(F("[HaBridge] ERROR: Failed to configure Ethernet via DHCP!"));
        // Continue anyway — Ethernet.maintain() may recover later
    }

    Serial.print(F("[HaBridge] IP address: "));
    Serial.println(Ethernet.localIP());

    // ── Configure HA Device ─────────────────────────
    device_.setName(config::kDeviceName);
    device_.setSoftwareVersion(config::kSoftwareVersion);
    device_.setManufacturer(config::kManufacturer);
    device_.enableLastWill();

    // ── Configure Pump Switch ───────────────────────
    pumpSwitch_.setName(config::kPumpName);
    pumpSwitch_.setIcon("mdi:water-pump");
    pumpSwitch_.onCommand(onPumpCommand);

    // ── Configure Zone Switches ─────────────────────
    for (uint8_t i = 0; i < config::kZoneCount; ++i) {
        zoneSwitches_[i].setName(config::kZoneNames[i]);
        // Tap (index 5) gets a faucet icon, zones get sprinkler
        zoneSwitches_[i].setIcon(i == 5 ? "mdi:faucet" : "mdi:sprinkler");
        zoneSwitches_[i].onCommand(onZoneCommand);
    }

    // ── Begin MQTT ──────────────────────────────────
    if (MQTT_USER != nullptr && strlen(MQTT_USER) > 0) {
        mqtt_.begin(MQTT_BROKER_IP, MQTT_BROKER_PORT, MQTT_USER, MQTT_PASSWORD);
    } else {
        mqtt_.begin(MQTT_BROKER_IP, MQTT_BROKER_PORT);
    }

    // ── Subscribe to ping topic for health check ────
    mqtt_.onMessage(onMqttMessage);

    Serial.println(F("[HaBridge] Initialized."));
}

void HaBridge::update() {
    Ethernet.maintain();   // Renew DHCP lease if needed
    mqtt_.loop();          // Process MQTT messages, auto-reconnect

    // MQTT round-trip health check (feeds watchdog)
    checkMqttHealth();

    // Sync relay state → HA (only when changed)
    syncStates();
}

// ── Static Callbacks ────────────────────────────────

void HaBridge::onZoneCommand(bool state, HASwitch* sender) {
    auto& relays = RelayController::instance();

    // Identify which zone switch fired
    for (uint8_t i = 0; i < config::kZoneCount; ++i) {
        if (sender == &instance().zoneSwitches_[i]) {
            if (state) {
                relays.turnOnZone(i);
            } else {
                relays.turnOffZone(i);
            }
            sender->setState(state);
            return;
        }
    }
}

void HaBridge::onPumpCommand(bool state, HASwitch* sender) {
    auto& relays = RelayController::instance();

    if (state) {
        // Pump must never run without an open zone — reject the command
        if (!relays.hasAnyZoneOn()) {
            sender->setState(false);   // report back: pump stays off
            return;
        }
        relays.turnOnPump();
        sender->setState(true);
    } else {
        relays.turnOffPump();          // master kill: pump + all zones off
        sender->setState(false);
    }
}

// ── MQTT Round-Trip Health Check ────────────────────

void HaBridge::checkMqttHealth() {
    // Subscribe to our ping topic on first successful connection
    if (mqtt_.isConnected() && !pingSubscribed_) {
        mqtt_.subscribe("vizrendszer/ping");
        pingSubscribed_ = true;
    }
    if (!mqtt_.isConnected()) {
        pingSubscribed_ = false;
        return;
    }

    unsigned long now = millis();

    // Send a ping every kMqttPingIntervalMs (payload = unique millis ID)
    if (now - lastPingSentMs_ >= config::kMqttPingIntervalMs) {
        snprintf(pingId_, sizeof(pingId_), "%lu", now);
        mqtt_.publish("vizrendszer/ping", pingId_);
        lastPingSentMs_ = now;
    }

    // Report health to watchdog: round-trip must be within kMqttPongTimeoutMs
    bool healthy = (lastPongReceivedMs_ > 0) && (now - lastPongReceivedMs_ < config::kMqttPongTimeoutMs);
    Watchdog::setHealthy(healthy);
}

bool HaBridge::isMqttHealthy() const {
    if (lastPongReceivedMs_ == 0) return false;
    return (millis() - lastPongReceivedMs_ < config::kMqttPongTimeoutMs);
}

// ── Static Callbacks ────────────────────────────────

void HaBridge::onMqttMessage(const char* topic, const uint8_t* payload, unsigned int length) {
    (void)payload;
    (void)length;
    // Check if this is our own ping coming back via the broker (round-trip)
    if (strcmp(topic, "vizrendszer/ping") == 0) {
        // Any message on this topic confirms bidirectional MQTT is alive
        instance().lastPongReceivedMs_ = millis();
    }
}

// ── State Sync ──────────────────────────────────────

void HaBridge::syncStates() {
    // Sync pump state
    bool pumpOn = RelayController::instance().isPumpOn();
    if (pumpOn != lastPumpState_) {
        pumpSwitch_.setState(pumpOn);
        lastPumpState_ = pumpOn;
    }

    // Sync zone states
    for (uint8_t i = 0; i < config::kZoneCount; ++i) {
        bool zoneOn = RelayController::instance().isZoneOn(i);
        if (zoneOn != lastZoneStates_[i]) {
            zoneSwitches_[i].setState(zoneOn);
            lastZoneStates_[i] = zoneOn;
        }
    }
}
