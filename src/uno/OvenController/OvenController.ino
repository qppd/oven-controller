// Main Arduino sketch for OvenController

#include <stdio.h>
#include "LCD_CONF.h"
#include "UI_CONF.h"
#include "DS3231_CONF.h"
#include "TACTILE_CONF.h"
#include "RELAY_CONF.h"
#include "DHT_CONF.h"
#include "MAX6675_CONF.h"
#include "PID_CONF.h"

// -------------------------------------------------------------------
// Oven state machine definitions
// -------------------------------------------------------------------
// OvenState is now defined in UI_CONF.h
OvenState currentState = IDLE;

int timerMinutes = 0; // timer duration in minutes
AlarmId ovenTimer = dtINVALID_ALARM_ID; // track oven timer

// Sensors
DHT22Sensor dhtSensor(DHT22_PIN);
MAX6675Sensor tempSensor;

// Timing control for non-blocking updates
static unsigned long lastSensorRead = 0;     // Track sensor read timing
static unsigned long lastLCDUpdate = 0;      // Track LCD update timing
const unsigned long SENSOR_INTERVAL = 1000;  // Read sensors every 1000ms (1 sec)
const unsigned long LCD_INTERVAL = 500;      // Update LCD every 500ms (non-critical flicker fix)
static OvenState lastRenderedState = ERROR;  // Track last rendered state for change detection

// Temperature averaging - 10-reading circular buffer
const int TEMP_BUFFER_SIZE = 10;
static float tempBuffer[TEMP_BUFFER_SIZE] = {0.0};
static int tempBufferIndex = 0;
static int tempBufferFilled = 0;  // Count of actual readings
static float averagedTemperature = 0.0;

// Timer tracking for RUNNING state
static unsigned long ovenStartTime = 0;  // Timestamp when oven started
static unsigned long timerDurationMs = 0; // Total timer duration in milliseconds

// -------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------
void setup() {
    Serial.begin(9600);
    while (!Serial) {;}

    // LCD
    initLCD();
    initUI();
    clearLCD();
    setLCDText("Oven Ready", 0, 0);

    // Buttons
    initBUTTONS();

    // Relays
    initRelays();

    pinMode(LED_BUILTIN, OUTPUT);

    // RTC
    dsInitRTC();

    // Sensors
    dhtSensor.begin();
    tempSensor.begin();

    // PID
    initPID();
}

// -------------------------------------------------------------------
// Main Loop - FULLY NON-BLOCKING
// -------------------------------------------------------------------
void loop() {
    // Button input check - ALWAYS responsive, NO BLOCKING
    setInputFlags();
    resolveInputFlags();

    // RTC handling - non-blocking alarm engine
    dsHandleRTC();

    // Sensor reads and temperature processing - THROTTLED to 1 second
    unsigned long currentMillis = millis();
    if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = currentMillis;

        // Read temperature from MAX6675 and add to averaging buffer
        float rawTemperature = tempSensor.readTemperature();
        
        // Add to circular buffer for averaging
        tempBuffer[tempBufferIndex] = rawTemperature;
        tempBufferIndex = (tempBufferIndex + 1) % TEMP_BUFFER_SIZE;
        if (tempBufferFilled < TEMP_BUFFER_SIZE) {
            tempBufferFilled++;
        }
        
        // Calculate average of buffered readings
        float sum = 0.0;
        for (int i = 0; i < tempBufferFilled; i++) {
            sum += tempBuffer[i];
        }
        averagedTemperature = sum / tempBufferFilled;
        CURRENT_TEMPERATURE = averagedTemperature;
        
        Serial.print("Temperature (raw): ");
        Serial.print(rawTemperature);
        Serial.print(" (avg): ");
        Serial.println(averagedTemperature);

        // Read humidity from DHT22
        float humidity = dhtSensor.readHumidity();
        Serial.print("Humidity: ");
        Serial.println(humidity);

        // Safety cutoff - only check on sensor read (use averaged temp)
        if (averagedTemperature > 130.0 || averagedTemperature < 0) {
            stopOven();
            currentState = ERROR;
        } else if (currentState == RUNNING) {
            // PID computation ONLY when oven is actually running
            computePID();
        }
    }

    // LCD rendering - THROTTLED to avoid flickering + on state change
    if ((currentMillis - lastLCDUpdate >= LCD_INTERVAL) || (currentState != lastRenderedState)) {
        lastLCDUpdate = currentMillis;

        // Trigger full refresh on state change
        if (currentState != lastRenderedState) {
            forceUIRefresh();
        }
        lastRenderedState = currentState;

        // Calculate remaining timer seconds
        unsigned long remainingSec = 0;
        if (currentState == RUNNING) {
            unsigned long elapsedMs = currentMillis - ovenStartTime;
            if (timerDurationMs > elapsedMs) {
                remainingSec = (timerDurationMs - elapsedMs) / 1000;
            }
        } else if (currentState == SET_TEMP || currentState == SET_TIMER || currentState == READY || currentState == IDLE) {
            remainingSec = (unsigned long)timerMinutes * 60UL;
        }

        updateUI(currentState, averagedTemperature, dhtSensor.getLastHumidity(),
                 TEMPERATURE_SETPOINT, timerMinutes, remainingSec);
    }
    
    // No delay() - loop runs as fast as possible for responsive input
}

// -------------------------------------------------------------------
// Button input handler
// -------------------------------------------------------------------
void inputAction(int index) {
    switch (currentState) {

        case IDLE:
            if (index == 2) currentState = SET_TEMP;
            break;

        case SET_TEMP:
            if (index == 0) TEMPERATURE_SETPOINT += 5;
            if (index == 1) TEMPERATURE_SETPOINT -= 5;
            if (index == 2) currentState = SET_TIMER;

            TEMPERATURE_SETPOINT = constrain(TEMPERATURE_SETPOINT, 60, 125);
            break;

        case SET_TIMER:
            if (index == 0) timerMinutes += 5;
            if (index == 1) timerMinutes -= 5;
            if (index == 2) currentState = READY;

            timerMinutes = constrain(timerMinutes, 1, 180);
            break;

        case READY:
            if (index == 2) {
                startOven();
                currentState = RUNNING;
            }
            break;

        case RUNNING:
            if (index == 2) {
                stopOven();
                currentState = IDLE;
            }
            break;

        case FINISHED:
            if (index == 2) currentState = IDLE;
            break;

        case ERROR:
            if (index == 2) currentState = IDLE;
            break;
    }
}

// -------------------------------------------------------------------
// Oven control
// -------------------------------------------------------------------
void ovenStopCallback() {
    setHeaterRelay(false);
    setExhaustRelay(false);
    currentState = FINISHED;
}

void startOven() {
    setExhaustRelay(true);
    
    // Track timer start time and duration for real-time display
    ovenStartTime = millis();
    timerDurationMs = (unsigned long)timerMinutes * 60000UL;  // Convert minutes to milliseconds

    ovenTimer = Alarm.timerOnce(timerMinutes * 60, ovenStopCallback);
}

void stopOven() {
    setHeaterRelay(false);
    setExhaustRelay(false);

    if (ovenTimer != dtINVALID_ALARM_ID) {
        Alarm.free(ovenTimer);
        ovenTimer = dtINVALID_ALARM_ID;
    }
}

// -------------------------------------------------------------------
// Helper Functions for Timer and Display
// -------------------------------------------------------------------
void getTimerDisplay(char* buffer, int bufferSize) {
    unsigned long elapsedMs = millis() - ovenStartTime;
    unsigned long remainingMs = (timerDurationMs > elapsedMs) ? (timerDurationMs - elapsedMs) : 0;
    
    int remainingMinutes = remainingMs / 60000;
    int remainingSeconds = (remainingMs % 60000) / 1000;
    
    snprintf(buffer, bufferSize, "%02d:%02d", remainingMinutes, remainingSeconds);
}

void getTemperatureDisplay(char* buffer, int bufferSize) {
    snprintf(buffer, bufferSize, "%.1f C", CURRENT_TEMPERATURE);
}

// -------------------------------------------------------------------
// LCD Rendering - Improved layout with proper field sizing
// -------------------------------------------------------------------
void renderLCD() {
    static char tempStr[10];
    static char timerStr[10];
    
    switch (currentState) {

        case IDLE:
            // Row 0: "Temp: XX.X C"
            setLCDText("Temp: ", 0, 0);
            getTemperatureDisplay(tempStr, sizeof(tempStr));
            setLCDText(String(tempStr), 6, 0);
            setLCDText("    ", 12, 0);  // Clear rest of line

            // Row 1: Empty
            setLCDText("                    ", 0, 1);

            // Row 2: "Set:  XX C"
            setLCDText("Set:  ", 0, 2);
            setLCDText(TEMPERATURE_SETPOINT, 6, 2);
            setLCDText(" C      ", 8, 2);

            // Row 3: "Press SELECT"
            setLCDText("Press SELECT        ", 0, 3);
            break;

        case SET_TEMP:
            // Row 0: "SET TEMPERATURE"
            setLCDText("SET TEMPERATURE     ", 0, 0);

            // Row 1: "Target: XX C"
            setLCDText("Target: ", 0, 1);
            setLCDText(TEMPERATURE_SETPOINT, 8, 1);
            setLCDText(" C          ", 10, 1);

            // Row 2: "BTN1:+  BTN2:-"
            setLCDText("BTN1:+  BTN2:-      ", 0, 2);

            // Row 3: "BTN3: NEXT"
            setLCDText("BTN3: NEXT          ", 0, 3);
            break;

        case SET_TIMER:
            // Row 0: "SET TIMER"
            setLCDText("SET TIMER           ", 0, 0);

            // Row 1: "Duration: XX min"
            setLCDText("Duration: ", 0, 1);
            setLCDText(timerMinutes, 10, 1);
            setLCDText(" min    ", 12, 1);

            // Row 2: "BTN1:+  BTN2:-"
            setLCDText("BTN1:+  BTN2:-      ", 0, 2);

            // Row 3: "BTN3: CONFIRM"
            setLCDText("BTN3: CONFIRM       ", 0, 3);
            break;

        case READY:
            // Row 0: "READY FOR START"
            setLCDText("READY FOR START     ", 0, 0);

            // Row 1: "Target: XX C"
            setLCDText("Target: ", 0, 1);
            setLCDText(TEMPERATURE_SETPOINT, 8, 1);
            setLCDText(" C          ", 10, 1);

            // Row 2: "Duration: XX min"
            setLCDText("Duration: ", 0, 2);
            setLCDText(timerMinutes, 10, 2);
            setLCDText(" min    ", 12, 2);

            // Row 3: "BTN3: START"
            setLCDText("BTN3: START         ", 0, 3);
            break;

        case RUNNING:
            // Row 0: "HEATING"
            setLCDText("HEATING             ", 0, 0);

            // Row 1: "Temp: XX.X / XX C"
            setLCDText("Temp: ", 0, 1);
            getTemperatureDisplay(tempStr, sizeof(tempStr));
            setLCDText(String(tempStr), 6, 1);
            setLCDText(" / ", 12, 1);
            setLCDText(TEMPERATURE_SETPOINT, 15, 1);
            setLCDText(" C", 17, 1);

            // Row 2: "Time: MM:SS"
            setLCDText("Time: ", 0, 2);
            getTimerDisplay(timerStr, sizeof(timerStr));
            setLCDText(String(timerStr), 6, 2);
            setLCDText("          ", 11, 2);

            // Row 3: "BTN3: STOP"
            setLCDText("BTN3: STOP          ", 0, 3);
            break;

        case FINISHED:
            // Row 0: "COMPLETE!"
            setLCDText("COMPLETE!           ", 0, 0);

            // Row 1: "Cycle finished"
            setLCDText("Cycle finished      ", 0, 1);

            // Row 2: Empty
            setLCDText("                    ", 0, 2);

            // Row 3: "BTN3: RESET"
            setLCDText("BTN3: RESET         ", 0, 3);
            break;

        case ERROR:
            // Row 0: "ERROR!"
            setLCDText("ERROR!              ", 0, 0);

            // Row 1: "Overtemp or fault"
            setLCDText("Overtemp or fault   ", 0, 1);

            // Row 2: "Temp: XX.X C"
            setLCDText("Temp: ", 0, 2);
            getTemperatureDisplay(tempStr, sizeof(tempStr));
            setLCDText(String(tempStr), 6, 2);
            setLCDText("     ", 12, 2);

            // Row 3: "BTN3: RESET"
            setLCDText("BTN3: RESET         ", 0, 3);
            break;
    }
}