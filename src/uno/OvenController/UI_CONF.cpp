// Non-blocking LCD UI implementation for 20x4 I2C display.
#include "UI_CONF.h"
#include "LCD_CONF.h"
#include "PINS_CONF.h"
#include <TimeLib.h>
#include <stdio.h>
#include <string.h>

// Global relay state tracking
bool heaterRelayOn = false;
bool exhaustRelayOn = false;

// Cache for dirty-row detection (20 chars + null terminator)
static char lastRow0[21] = "";
static char lastRow1[21] = "";
static char lastRow2[21] = "";
static char lastRow3[21] = "";

// Blink timing
static unsigned long lastBlinkToggle = 0;
static bool blinkVisible = true;

// RTC time caching
static unsigned long lastTimeUpdate = 0;
static char cachedTimeStr[9] = "--:--:--";

// Force redraw flag
static bool forceRefresh = false;

// --- Internal Helpers ---

static const char* stateToLabel(OvenState state) {
    switch (state) {
    case IDLE:      return "IDLE";
    case SET_TEMP:   return "SET T";
    case SET_TIMER:  return "SET TM";
    case READY:     return "READY";
    case RUNNING:   return "RUN";
    case FINISHED:  return "DONE";
    case ERROR:     return "ERR";
    default:        return "???";
}
}

static void formatCurrentTime(char* out) {
    if (timeStatus() == timeNotSet) {
        strcpy(out, "--:--:--");
    } else {
        snprintf(out, 9, "%02d:%02d:%02d", hour(), minute(), second());
    }
}

static void writeRow(int row, const char* buf) {
    char safe[21];
    memset(safe, ' ', 20);
    safe[20] = '\0';

    // Copy provided buffer, truncated to 20
    int len = strlen(buf);
    if (len > 20) len = 20;
    memcpy(safe, buf, len);

    char* last = NULL;
    switch (row) {
        case 0: last = lastRow0; break;
        case 1: last = lastRow1; break;
        case 2: last = lastRow2; break;
        case 3: last = lastRow3; break;
        default: return;
    }

    if (!forceRefresh && strcmp(safe, last) == 0) return;

    strcpy(last, safe);
    lcd.setCursor(0, row);
    lcd.print(safe);
}

// --- Public API ---

void initUI() {
    forceRefresh = true;
    memset(lastRow0, 0, 21);
    memset(lastRow1, 0, 21);
    memset(lastRow2, 0, 21);
    memset(lastRow3, 0, 21);
}

void forceUIRefresh() {
    forceRefresh = true;
}

void updateUI(OvenState state, float temperature, float humidity,
              int setpoint, int timerMinutes, unsigned long remainingSec) {

    // Read actual hardware states for SSR indicators
    heaterRelayOn = (digitalRead(RELAY_HEATER_PIN) == HIGH);
    exhaustRelayOn = (digitalRead(RELAY_EXHAUST_PIN) == HIGH);

    // Blink trigger every 500ms
    if (millis() - lastBlinkToggle >= 500) {
        lastBlinkToggle = millis();
        blinkVisible = !blinkVisible;
    }

    // Refresh RTC time string every second
    if (millis() - lastTimeUpdate >= 1000) {
        lastTimeUpdate = millis();
        formatCurrentTime(cachedTimeStr);
    }

    char buf[32];

    // LINE 0: SSR Status + Oven State
    // H:ON E:OFF RUN
    const char* hS = heaterRelayOn ? "ON" : "OFF";
    const char* eS = exhaustRelayOn ? "ON" : "OFF";
    snprintf(buf, sizeof(buf), "H:%-3s E:%-3s %s", hS, eS, stateToLabel(state));
    writeRow(0, buf);

    // LINE 1: Temp (Actual / Setpoint)
    // T:085C S:120C
    int tInt = (int)(temperature + 0.5f);
    if (state == SET_TEMP && !blinkVisible) {
        snprintf(buf, sizeof(buf), "T:%03dC S:---C", tInt);
    } else {
        snprintf(buf, sizeof(buf), "T:%03dC S:%03dC", tInt, setpoint);
    }
    writeRow(1, buf);

    // LINE 2: Humidity only (RTC time removed)
    // H:065%
    int hInt = (int)(humidity + 0.5f);
    snprintf(buf, sizeof(buf), "Humidity: %03d%%", hInt);
    writeRow(2, buf);

    // LINE 3: Timer + State Label
    // TMR:01:25:40 RUN
    unsigned long s = remainingSec;
    const char* stateLabel = stateToLabel(state);

    unsigned int hh = s / 3600;
    unsigned int mm = (s % 3600) / 60;
    unsigned int ss = s % 60;

    if (state == SET_TIMER && !blinkVisible) {
        snprintf(buf, sizeof(buf), "TMR:--:--:-- %s", stateLabel);
    } else {
        snprintf(buf, sizeof(buf), "TMR:%02u:%02u:%02u %s", hh, mm, ss, stateLabel);
    }
    writeRow(3, buf);

    forceRefresh = false;
}
