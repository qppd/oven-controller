// Relay configuration implementation for heater and exhaust control.
#include <Arduino.h>
#include "RELAY_CONF.h"

void initRelays() {
    pinMode(RELAY_HEATER_PIN, OUTPUT);
    pinMode(RELAY_EXHAUST_PIN, OUTPUT);
    // Start with relays off for safety.
    digitalWrite(RELAY_HEATER_PIN, LOW);
    digitalWrite(RELAY_EXHAUST_PIN, LOW);
}

void setHeaterRelay(bool on) {
    digitalWrite(RELAY_HEATER_PIN, on ? HIGH : LOW);
}

void setExhaustRelay(bool on) {
    digitalWrite(RELAY_EXHAUST_PIN, on ? HIGH : LOW);
}
