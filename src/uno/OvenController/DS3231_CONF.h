#pragma once
#ifndef DS3231_CONF_H
#define DS3231_CONF_H

// ---------- Core libraries ----------
#include <Wire.h>
#include <RTClib.h>          // defines RTC_DS3231
#include <TimeLib.h>         // provides time‑related functions (now(), setTime(), etc.)
#include <TimeAlarms.h>      // alarm/timer functions

// ---------- Global objects ----------
/*  The RTC instance is defined in the .cpp file.
    We only forward‑declare it here so the header can be included
    by multiple translation units without ODR violations. */
extern RTC_DS3231 rtc;

/*  Forward declarations of alarm callbacks that the oven logic
    will use.  Give them meaningful names – you can implement the
    bodies in another .cpp or directly in this file if you keep everything
    together. */
void ovenStartAlarm();      // called when the oven should turn on
void ovenStopAlarm();       // called when the oven should turn off
void ovenWeeklyMaintenance(); // optional weekly check

// ---------- Public API ----------
/*  Call this once from `setup()` (or wherever you start the controller).
    It begins the RTC, restores time on power‑loss, and seeds the alarm
    engine. */
void dsInitRTC();

/*  Call this as often as possible from `loop()`.  It:
      • advances the TimeAlarms engine,
      • periodically syncs the Arduino `time()` with the DS3231,
      • runs any scheduled oven alarms. */
void dsHandleRTC();

/*  Optional helper – if you need a raw timer callback (e.g., LED blink). */
void timerTriggered();

#endif // DS3231_CONF_H