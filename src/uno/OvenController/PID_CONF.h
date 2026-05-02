// PID configuration header – now implements a temperature PID controller.
#ifndef PID_CONF_H
#define PID_CONF_H

#include <PID_v1.h>

// -------------------------------------------------------------------
// Tuning parameters (adjust as needed for your oven hardware)
// -------------------------------------------------------------------
extern double P;
extern double I;
extern double D;

// -------------------------------------------------------------------
// Set‑points
// -------------------------------------------------------------------
extern double HUMIDITY_SETPOINT;      // placeholder for future humidity control
extern double TEMPERATURE_SETPOINT;

// -------------------------------------------------------------------
// Runtime variables
// -------------------------------------------------------------------
extern double CURRENT_TEMPERATURE;    // updated each loop from DS18B20
extern double outputTemperature;     // PID output for temperature
extern double outputHumidity;         // placeholder for future humidity PID

// -------------------------------------------------------------------
// PID objects
// -------------------------------------------------------------------
extern PID pidTemperature;

// -------------------------------------------------------------------
// Public API
// -------------------------------------------------------------------
/* Initialise the PID controller (set mode, limits, etc.). */
void initPID();

/* Compute the PID output and drive the relays accordingly. */
void computePID();

#endif // PID_CONF_H