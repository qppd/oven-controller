#pragma once
#include <Arduino.h>

// Pin assignments for OvenController hardware
// Relay 1 (heater & convection) -> pin 2
// Relay 2 (exhaust) -> pin 3
// Tactile buttons -> pins 4, 5, 6
// DS18B20 temperature sensor -> pin 7
// DHT22 humidity sensor -> pin 8
// LCD uses I2C (default SDA=A4, SCL=A5 on Arduino Uno)

const int RELAY_HEATER_PIN = 2;   // Relay 1 controls heater and convection
const int RELAY_EXHAUST_PIN = 3;  // Relay 2 controls exhaust fan

const int BUTTON_PIN_1 = 4;       // Tactile button 1
const int BUTTON_PIN_2 = 5;       // Tactile button 2
const int BUTTON_PIN_3 = 6;       // Tactile button 3

const int DS18B20_PIN = 7;        // Temperature sensor pin
const int DHT22_PIN   = 8;        // Humidity sensor pin

// I2C pins for LCD (LiquidCrystal_I2C library)
const int LCD_I2C_SDA = A4;
const int LCD_I2C_SCL = A5;
