#include "DS3231_CONF.h"

// ----- Definition of the global RTC object -----
RTC_DS3231 rtc;   // <-- this is the only definition; the header only forward‑declares it

/*--------------------------------------------------------------------
   dsInitRTC()
   • Starts the I²C bus that talks to the DS3231.
   • Initializes the RTC module.
   • If the RTC has lost power, sets it to the compile‑time date/time.
   • Starts the TimeAlarms Engine (required before any alarm calls).
--------------------------------------------------------------------*/
void dsInitRTC() {
    // I²C bus selection – AVR (UNO/Nano) uses the default Wire library
    #ifdef AVR
        Wire.begin();
    #else
        Wire1.begin();   // Due or other boards with a second I²C bus
    #endif

    // Basic RTC sanity check
    if (!rtc.begin()) {
        Serial.println(F("WARNING: RTC not found - using system time"));
        // Continue anyway - don't block, just log warning
    }

    // If the battery was removed or the time is off, set it now
    if (rtc.lostPower()) {
        Serial.println(F("RTC lost power, setting time to compile moment"));
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Initialize the TimeAlarms library
    // Note: TimeAlarms doesn't require explicit initialization in newer versions
    // but we can set initial time if needed
    setTime(rtc.now().hour(), rtc.now().minute(), rtc.now().second(),
             rtc.now().day(), rtc.now().month(), rtc.now().year());
}

/*--------------------------------------------------------------------
   dsHandleRTC()
   • Non-blocking advancement of the TimeAlarms scheduler.
   • Every second syncs the Arduino `time()` with the actual DS3231 time
     so the `now()` function stays accurate.
--------------------------------------------------------------------*/
void dsHandleRTC() {
    // Non-blocking alarm engine advancement
    Alarm.delay(0);   // Process pending alarms without blocking

    // Periodic resynchronisation with the hardware RTC every ~60 seconds
    static unsigned long lastSync = millis();
    if (millis() - lastSync > 60000) {
        DateTime nowFromRTC = rtc.now();
        setTime(nowFromRTC.hour(), nowFromRTC.minute(), nowFromRTC.second(),
                nowFromRTC.day(), nowFromRTC.month(), nowFromRTC.year());
        lastSync = millis();
    }
}

/*--------------------------------------------------------------------
   timerTriggered()
   • Example callback used by the original sketch (LED toggle, buzzer, etc.).
   • Keep it empty or add your own logic if you need a periodic interrupt.
--------------------------------------------------------------------*/
void timerTriggered() {
    // Place any one‑off timer code here.
    // Example: toggle the built‑in LED for visual feedback.
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}