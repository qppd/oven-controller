// DS18B20 temperature sensor implementation.
#include "DS18B20_CONF.h"

DS18B20Sensor::DS18B20Sensor(uint8_t pin) : oneWire(pin), sensors(&oneWire) {}

void DS18B20Sensor::begin() {
    sensors.begin();
}

float DS18B20Sensor::readTemperature() {
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}
