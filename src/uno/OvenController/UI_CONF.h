// Non-blocking LCD UI module for 20x4 I2C display.
// Uses partial row updates, fixed-width formatting, and dirty-detection
// to eliminate flicker and I2C spam.
#ifndef UI_CONF_H
#define UI_CONF_H

#include <Arduino.h>

// Must match the enum in OvenController.ino
enum OvenState { IDLE, SET_TEMP, SET_TIMER, READY, RUNNING, FINISHED, ERROR };

// Relay state tracking — set by the main loop whenever relays change
extern bool heaterRelayOn;
extern bool exhaustRelayOn;

// Call once from setup()
void initUI();

// Call from loop() — non-blocking, partial-update LCD renderer.
// remainingSec = live countdown seconds (only meaningful in RUNNING).
void updateUI(OvenState state, float temperature, float humidity,
              int setpoint, int timerMinutes, unsigned long remainingSec);

// Force next updateUI() to redraw all rows (e.g. after state change)
void forceUIRefresh();

#endif