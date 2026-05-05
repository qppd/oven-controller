// DHT sensor implementation (humidity only) - NON-BLOCKING.
#include "DHT_CONF.h"

DHT22Sensor::DHT22Sensor(uint8_t pin) : dht(pin, DHT22), sensorPin(pin) {}

void DHT22Sensor::begin() {
    dht.begin();
}

float DHT22Sensor::readHumidity() {
    // DHT requires 2+ seconds between reads - only read if enough time passed
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= MIN_READ_INTERVAL) {
        lastReadTime = currentTime;
        float humidity = dht.readHumidity();
        
        // Only update if valid reading (DHT returns NAN on error)
        if (!isnan(humidity)) {
            lastHumidity = humidity;
        }
    }
    
    // Return cached value (non-blocking)
    return lastHumidity;
}

float DHT22Sensor::getLastHumidity() {
    // Get cached value without any I2C operation
    return lastHumidity;
}