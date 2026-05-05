#ifndef MAX6675_CONF_H
#define MAX6675_CONF_H

#include <Arduino.h>
#include "max6675.h"
#include "PINS_CONF.h"

// MAX6675 Pins are defined in PINS_CONF.h
// #define THERMO_DO   4
// #define THERMO_CS   5
// #define THERMO_CLK  6

class MAX6675Sensor {
public:
    MAX6675Sensor();
    void begin();
    float readTemperature();   // Celsius
    float readFahrenheit();    // Fahrenheit

private:
    MAX6675 thermocouple;
};

#endif