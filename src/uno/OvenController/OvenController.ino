// Main Arduino sketch for OvenController.
#include "LCD_CONF.h"
#include "DS3231_CONF.h"
#include "TACTILE_CONF.h"
#include "RELAY_CONF.h"
#include "DHT_CONF.h"
#include "DS18B20_CONF.h"
#include "PID_CONF.h"

// -------------------------------------------------------------------
// Oven state machine definitions
// -------------------------------------------------------------------
enum OvenState { IDLE, SET_TEMP, SET_TIMER, READY, RUNNING, FINISHED, ERROR };
OvenState currentState = IDLE;
int timerMinutes = 0; // timer duration in minutes
AlarmId ovenTimer = dtINVALID_ALARM_ID; // track oven timer


// Global sensor instances using pins from PINS_CONF.h
DHT22Sensor dhtSensor(DHT22_PIN);
DS18B20Sensor tempSensor(DS18B20_PIN);

void setup() {
    // Initialize serial for debugging / output
    Serial.begin(9600);
    while (!Serial) { ; } // wait for serial port to connect (optional)

    // Initialize LCD display
    initLCD();
    clearLCD();
    setLCDText("Oven Ready", 0, 0);

    // Initialize tactile buttons
    initBUTTONS();

    // Initialize relays (heater and exhaust)
    initRelays();
    // Initialize built‑in LED for timer visual feedback
    pinMode(LED_BUILTIN, OUTPUT);
    // Initialize DS3231 RTC and timers
    dsInitRTC();

    // Initialize sensors
    dhtSensor.begin();
    tempSensor.begin();

    // Initialize PID controller
    initPID();
}

void loop() {
    // Update button states and handle actions
    setInputFlags();
    resolveInputFlags();

    // Read humidity and output to serial
    float humidity = dhtSensor.readHumidity();
    Serial.print("Humidity: ");
    Serial.println(humidity);

    // Read temperature, update PID, and output to serial
    float temperature = tempSensor.readTemperature();
    // Update global temperature for PID controller
    CURRENT_TEMPERATURE = temperature;
    // Safety: over‑temperature cutoff
    if (temperature > 130.0) {
        stopOven();
        currentState = ERROR;
    } else {
        computePID();
    }
    Serial.print("Temperature: ");
    Serial.println(temperature);

    // Small delay to avoid saturating the CPU
        // Process DS3231 alarms and periodic sync
    dsHandleRTC();
    // Update LCD based on current state
    renderLCD();
    delay(10);
}

// Button input action handler
void inputAction(int index) {
    // (implementation as above)
    switch (currentState) {
        case IDLE:
            if (index == 2) {
                currentState = SET_TEMP;
            }
            break;
        case SET_TEMP:
            if (index == 0) {
                TEMPERATURE_SETPOINT++;
            } else if (index == 1) {
                TEMPERATURE_SETPOINT--;
            } else if (index == 2) {
                currentState = SET_TIMER;
            }
            // Constrain setpoint range (60‑125°C)
            TEMPERATURE_SETPOINT = constrain(TEMPERATURE_SETPOINT, 60, 125);
            break;
        case SET_TIMER:
            if (index == 0) {
                timerMinutes++;
            } else if (index == 1) {
                timerMinutes--;
            } else if (index == 2) {
                currentState = READY;
            }
            // Clamp timer to 1‑180 min
            timerMinutes = max(1, min(timerMinutes, 180));
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
            if (index == 2) {
                currentState = IDLE;
            }
            break;
        case ERROR:
            // Allow reset via SELECT button
            if (index == 2) {
                currentState = IDLE;
            }
            break;
    }
}

// -------------------------------------------------------------------
// Oven control helper functions (PID, timer, LCD)
// -------------------------------------------------------------------

void ovenStopCallback() {
    setHeaterRelay(false);
    setExhaustRelay(false);
    currentState = FINISHED;
}

void startOven() {
    // Turn on exhaust for safety, then heater via PID loop
    setExhaustRelay(true);
    // Schedule stop after timerMinutes and keep the timer ID
    ovenTimer = Alarm.timerOnce(timerMinutes * 60, ovenStopCallback);
}

void stopOven() {
    setHeaterRelay(false);
    setExhaustRelay(false);
    // Cancel the specific oven timer if it was set
    if (ovenTimer != dtINVALID_ALARM_ID) {
        Alarm.free(ovenTimer);
        ovenTimer = dtINVALID_ALARM_ID;
    }
}

void renderLCD() {
    clearLCD();
    switch (currentState) {
        case IDLE:
            setLCDText("Temp: ", 0, 0);
            setLCDText(CURRENT_TEMPERATURE, 6, 0);
            setLCDText("C", 10, 0);
            setLCDText("Set: ", 0, 2);
            setLCDText(TEMPERATURE_SETPOINT, 6, 2);
            setLCDText("C", 10, 2);
            setLCDText("Press SELECT", 0, 3);
            break;
        case SET_TEMP:
            setLCDText("SET TEMP", 0, 0);
            setLCDText(TEMPERATURE_SETPOINT, 0, 1);
            setLCDText("C", 3, 1);
            setLCDText("BTN1:+  BTN2:-", 0, 2);
            setLCDText("BTN3: NEXT", 0, 3);
            break;
        case SET_TIMER:
            setLCDText("SET TIMER", 0, 0);
            setLCDText(timerMinutes, 0, 1);
            setLCDText("min", 3, 1);
            setLCDText("BTN1:+  BTN2:-", 0, 2);
            setLCDText("BTN3: CONFIRM", 0, 3);
            break;
        case READY:
            setLCDText("READY", 0, 0);
            setLCDText(TEMPERATURE_SETPOINT, 0, 1);
            setLCDText("C", 3, 1);
            setLCDText(timerMinutes, 0, 2);
            setLCDText("min", 3, 2);
            setLCDText("BTN3: START", 0, 3);
            break;
        case RUNNING:
            setLCDText("RUNNING", 0, 0);
            setLCDText(CURRENT_TEMPERATURE, 0, 1);
            setLCDText("C", 5, 1);
            setLCDText("/", 6, 1);
            setLCDText(TEMPERATURE_SETPOINT, 7, 1);
            setLCDText("C", 10, 1);
            setLCDText("Timer", 0, 2);
            setLCDText("BTN3: STOP", 0, 3);
            break;
        case FINISHED:
            setLCDText("DONE!", 0, 0);
            setLCDText("Time Complete", 0, 1);
            setLCDText("BTN3: RESET", 0, 3);
            break;
        case ERROR:
            setLCDText("ERROR", 0, 0);
            setLCDText("Overtemp", 0, 1);
            setLCDText("BTN3: RESET", 0, 3);
            break;
    }
}
