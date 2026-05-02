// DHT sensor implementation (humidity only).
#include "DHT_CONF.h"

DHT22Sensor::DHT22Sensor(uint8_t pin) : dht(pin, DHT22), sensorPin(pin) {}

void DHT22Sensor::begin() {
    dht.begin();
}

float DHT22Sensor::readHumidity() {
    return dht.readHumidity();
}