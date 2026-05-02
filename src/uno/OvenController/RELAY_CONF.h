// Relay configuration header for controlling heater and exhaust.
#ifndef RELAY_CONF_H
#define RELAY_CONF_H

#include "PINS_CONF.h"

// Initialize relay pins as outputs and set them to a known safe state.
void initRelays();

// Control functions for each relay.
void setHeaterRelay(bool on);
void setExhaustRelay(bool on);

#endif // RELAY_CONF_H