// Tactile (button) configuration implementation.
#include "TACTILE_CONF.h"

// Variable definitions.
int inputState[BUTTON_COUNT];
int lastInputState[BUTTON_COUNT] = { LOW, LOW, LOW };
bool inputFlags[BUTTON_COUNT] = { LOW, LOW, LOW };
unsigned long lastDebounceTime[BUTTON_COUNT] = { 0, 0, 0 };
const int inputPins[BUTTON_COUNT] = { BUTTON_PIN_1, BUTTON_PIN_2, BUTTON_PIN_3 };

bool first_time = true;

void initBUTTONS() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        pinMode(inputPins[i], INPUT);
        digitalWrite(inputPins[i], HIGH); // enable internal pull‑up
    }
    // No delay - buttons ready immediately
    Serial.println("Push Buttons: Initialized!");
}

void setInputFlags() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        int reading = digitalRead(inputPins[i]);
        if (reading != lastInputState[i]) {
            lastDebounceTime[i] = millis();
        }
        if (millis() - lastDebounceTime[i] > DEBOUNCE_DELAY) {
            if (reading != inputState[i]) {
                inputState[i] = reading;
                if (inputState[i] == HIGH) {
                    inputFlags[i] = HIGH;
                }
            }
        }
        lastInputState[i] = reading;
    }
}

void resolveInputFlags() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (inputFlags[i] == HIGH) {
            if (i == 2 && first_time) {
                // Skip first press of button 3 as per original logic.
                first_time = false;
            } else {
                inputAction(i); // Call external handler.
            }
            inputFlags[i] = LOW;
        }
    }
}