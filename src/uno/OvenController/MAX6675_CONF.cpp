#include "MAX6675_CONF.h"

// Initialize MAX6675 object using pin defines from PINS_CONF.h
MAX6675Sensor::MAX6675Sensor()
    : thermocouple(THERMO_CLK, THERMO_CS, THERMO_DO) {}

void MAX6675Sensor::begin() {
    Serial.println("Initializing MAX6675...");
    // No delay - MAX6675 is ready immediately
}

float MAX6675Sensor::readTemperature() {
    float t = thermocouple.readCelsius();

    // MAX6675 sometimes returns NAN on disconnect
    if (isnan(t)) {
        return -1.0;
    }

    return t;
}

float MAX6675Sensor::readFahrenheit() {
    return thermocouple.readFahrenheit();
}