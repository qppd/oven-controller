// Tactile (button) configuration header.
#ifndef TACTILE_CONF_H
#define TACTILE_CONF_H

#include <Arduino.h>
#include "PINS_CONF.h"

// Use three buttons defined in PINS_CONF.h.
#define BUTTON_COUNT 3
#define DEBOUNCE_DELAY 50

// External variable declarations.
extern int inputState[BUTTON_COUNT];
extern int lastInputState[BUTTON_COUNT];
extern bool inputFlags[BUTTON_COUNT];
extern unsigned long lastDebounceTime[BUTTON_COUNT];
extern const int inputPins[BUTTON_COUNT];

extern bool first_time;

// Function prototypes.
void initBUTTONS();
void setInputFlags();
void resolveInputFlags();

// External action handler defined elsewhere.
extern void inputAction(int index);

#endif // TACTILE_CONF_H