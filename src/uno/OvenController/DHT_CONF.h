// DHT sensor configuration header (humidity only).
#ifndef DHT_CONF_H
#define DHT_CONF_H

#include <Arduino.h>
#include <DHT.h>

class DHT22Sensor {
public:
    DHT22Sensor(uint8_t pin);
    void begin();
    float readHumidity();

private:
    DHT dht;
    uint8_t sensorPin;
};

#endif // DHT_CONF_H