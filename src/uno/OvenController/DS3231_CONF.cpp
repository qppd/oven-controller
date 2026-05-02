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
        // In a real project you might want to retry or signal an error
        Serial.println(F("Couldn't find RTC"));
        while (1) ;               // halt – you could replace with a flag
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
   • Advances the TimeAlarms scheduler (needs ~1 s per call).
   • Every minute (or whenever you like) syncs the Arduino `time()`
     with the actual DS3231 time so the `now()` function stays accurate.
--------------------------------------------------------------------*/
void dsHandleRTC() {
    // Move the alarm engine forward – roughly 1 second per iteration
    Alarm.delay(1000);   // you can make this smaller if you need finer granularity

    // Periodic resynchronisation with the hardware RTC.
    // Adjust the interval (here: 60 seconds) to suit your application.
    static unsigned long lastSync = 0;
    if (now() - lastSync > 60) {
        DateTime nowFromRTC = rtc.now();
        setTime(nowFromRTC.hour(), nowFromRTC.minute(), nowFromRTC.second(),
                nowFromRTC.day(), nowFromRTC.month(), nowFromRTC.year());
        lastSync = now();
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

/*--------------------------------------------------------------------
   Alarm callback implementations (the actual oven control).
   These are tiny examples – adapt them to your hardware (relays,
   solid‑state switches, etc.).  They demonstrate how you would wire
   the alarm functions that the user asked for.
--------------------------------------------------------------------*/
void ovenStartAlarm() {
    // Turn the oven (or its heating element) on.
    // Replace `YOUR_RELAY_PIN` with the actual pin you wired.
    const uint8_t YOUR_RELAY_PIN = 7;
    pinMode(YOUR_RELAY_PIN, OUTPUT);
    digitalWrite(YOUR_RELAY_PIN, HIGH);   // assuming HIGH = ON
    Serial.println(F("🔥 Oven START – heating element ON"));
}

void ovenStopAlarm() {
    // Turn the oven off.
    const uint8_t YOUR_RELAY_PIN = 7;
    digitalWrite(YOUR_RELAY_PIN, LOW);    // assuming LOW = OFF
    Serial.println(F("❄️ Oven STOP – heating element OFF"));
}

void ovenWeeklyMaintenance() {
    // Example: flash an LED or log a message once a week.
    Serial.println(F("🛠️ Weekly oven maintenance – check sensors & calibrate"));
}