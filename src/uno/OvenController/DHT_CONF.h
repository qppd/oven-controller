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
    float getLastHumidity();  // Get cached value without reading

private:
    DHT dht;
    uint8_t sensorPin;
    float lastHumidity = 0.0;
    unsigned long lastReadTime = 0;
    const unsigned long MIN_READ_INTERVAL = 2000;  // DHT needs 2+ seconds between reads
};

#endif // DHT_CONF_H