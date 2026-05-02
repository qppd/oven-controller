// PID configuration implementation.
#include <Arduino.h>
#include "PID_CONF.h"
#include "RELAY_CONF.h"

// -------------------------------------------------------------------
// Tuning parameters (default values)
// -------------------------------------------------------------------
double P = 4.0;
double I = 0.0;
double D = 22.0;

// -------------------------------------------------------------------
// Set‑points (default)
// -------------------------------------------------------------------
double HUMIDITY_SETPOINT = 0.0; // Placeholder for future use
double TEMPERATURE_SETPOINT = 100.0;

// -------------------------------------------------------------------
// Runtime variables
// -------------------------------------------------------------------
double CURRENT_TEMPERATURE = 0.0;
double outputTemperature = 0.0;
double outputHumidity = 0.0;

// -------------------------------------------------------------------
// PID object
// -------------------------------------------------------------------
PID pidTemperature(&CURRENT_TEMPERATURE, &outputTemperature, &TEMPERATURE_SETPOINT, P, I, D, DIRECT);

// -------------------------------------------------------------------
// Public API implementations
// -------------------------------------------------------------------
void initPID() {
    // Configure PID controller
    pidTemperature.SetMode(AUTOMATIC);
    // Output limits can be tuned; using 0‑255 matches typical PWM range
    pidTemperature.SetOutputLimits(0, 255);
    // Sample time defaults (ms) – can be adjusted if needed
    pidTemperature.SetSampleTime(1000);
}

void computePID() {
    // Compute PID output based on current temperature
    pidTemperature.Compute();
    // Windowed (time‑proportional) control for smoother heating
    const unsigned long windowSize = 1000; // 1 second window
    static unsigned long windowStart = 0;
    if (millis() - windowStart >= windowSize) {
        windowStart = millis();
    }
    // Output range is 0‑255; turn heater on proportionally within the window
    if (outputTemperature > ((millis() - windowStart) * 255UL / windowSize)) {
        setHeaterRelay(true);
    } else {
        setHeaterRelay(false);
    }
    // Humidity control placeholder – not used currently
}
