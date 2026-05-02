// DS18B20 temperature sensor configuration header.
#ifndef DS18B20_CONF_H
#define DS18B20_CONF_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class DS18B20Sensor {
public:
    DS18B20Sensor(uint8_t pin);
    void begin();
    float readTemperature();
private:
    OneWire oneWire;
    DallasTemperature sensors;
};

#endif // DS18B20_CONF_H