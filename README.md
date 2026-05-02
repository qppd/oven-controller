## Oven Controller – Arduino‑Uno Embedded System
*Compact, menu‑driven oven controller with temperature & humidity sensing, scheduling, and PID‑based heating.*

---

## Overview
The Arduino‑based Oven Controller runs on an **Arduino Uno (AVR)** and provides a complete solution for managing a small oven or drying chamber.

**Key features**
- Real‑time temperature (DS18B20) & humidity (DHT22) monitoring
- RTC‑driven scheduling & timer (DS3231 + TimeAlarms)
- Heater & exhaust fan control via two relays
- 20×4 I²C LCD UI with three tactile buttons for menu navigation
- PID temperature regulation (PID_v1) with configurable set‑point (60‑125 °C)
- Flexible architecture: each hardware block lives in its own `.h/.cpp` pair

**Typical use‑cases**
- Laboratory drying oven
- Small food‑processing oven
- DIY thermal curing chamber

---

## System Architecture
```
+-------------------+      +-------------------+      +-------------------+
|   Arduino Uno     | ---> |   Hardware       | ---> |   External Power |
| (main sketch)    |      | (sensors, relays) |      |   & Safety Devices|
+-------------------+      +-------------------+      +-------------------+

+-------------------+      +-------------------+      +-------------------+
|   LCD (I2C)       | <--> |   Buttons (3)    | <--> |   User Interaction|
+-------------------+      +-------------------+      +-------------------+

+-------------------+      +-------------------+      +-------------------+
|   PID Controller  | <--> |   Temperature    |      |   Heater Relay   |
+-------------------+      +-------------------+      +-------------------+

+-------------------+      +-------------------+      +-------------------+
|   RTC (DS3231)    | <--> |   Alarms/Timers  |      |   Exhaust Relay   |
+-------------------+      +-------------------+      +-------------------+
```
- **Modular code**: each peripheral (LCD, relays, buttons, DHT22, DS18B20, DS3231, PID) has its own header and implementation file.
- **Separation of concerns**: hardware initialization, UI rendering, sensor acquisition, and control logic are isolated, making extensions (e.g., Wi‑Fi, EEPROM) straightforward.

---

## Hardware Components
| Component | Model / Library | Purpose |
|----------|----------------|----------|
| Arduino Uno | ATmega328P | Core MCU |
| DS18B20 | OneWire + DallasTemperature | Temperature sensor (pin 7) |
| DHT22 | DHT library | Humidity sensor (pin 8) |
| DS3231 | RTClib | Real‑time clock & calendar |
| 2× Relay modules | Direct `digitalWrite` | Heater (pin 2) & Exhaust fan (pin 3) |
| 20×4 I²C LCD | LiquidCrystal_I2C (0x27) | UI display |
| 3× Tactile buttons | Pull‑up inputs (pins 4‑6) | Menu navigation (UP, DOWN, SELECT) |
| I²C bus lines | SDA=A4, SCL=A5 | LCD communication |

**Pin configuration** (see `PINS_CONF.h`)
| Pin | Function |
|-----|----------|
| 2   | Heater relay |
| 3   | Exhaust relay |
| 4   | Button 1 (UP) |
| 5   | Button 2 (DOWN) |
| 6   | Button 3 (SELECT) |
| 7   | DS18B20 temperature sensor |
| 8   | DHT22 humidity sensor |
| A4  | I²C SDA (LCD) |
| A5  | I²C SCL (LCD) |

---

## Software Architecture
| Module | Description |
|--------|-------------|
| **OvenController.ino** | Main sketch: state machine, `setup()` / `loop()`, UI rendering, button handling, safety checks. |
| **LCD_CONF** (`LCD_CONF.h/.cpp`) | Wraps `LiquidCrystal_I2C`; provides `initLCD()`, `clearLCD()`, and overloaded `setLCDText()` for various data types. |
| **RELAY_CONF** (`RELAY_CONF.h/.cpp`) | Initializes relay pins and exposes `setHeaterRelay(bool)` / `setExhaustRelay(bool)`. |
| **TACTILE_CONF** (`TACTILE_CONF.h/.cpp`) | Handles button debouncing, state flags, and forwards button events to `inputAction(int)`. |
| **DHT_CONF** (`DHT_CONF.h/.cpp`) | Simple wrapper class `DHT22Sensor` exposing `begin()` and `readHumidity()`. |
| **DS18B20_CONF** (`DS18B20_CONF.h/.cpp`) | Wrapper class `DS18B20Sensor` exposing `begin()` and `readTemperature()`. |
| **DS3231_CONF** (`DS3231_CONF.h/.cpp`) | Global `RTC_DS3231` instance, `dsInitRTC()` for init, `dsHandleRTC()` for alarm processing and periodic time sync. |
| **PID_CONF** (`PID_CONF.h/.cpp`) | PID tuning (`P=4`, `I=0`, `D=22`), set‑points, runtime variables, `initPID()` and `computePID()` that drive the heater relay via time‑proportional control. |

The **state machine** (`OvenState`) cycles through: `IDLE → SET_TEMP → SET_TIMER → READY → RUNNING → FINISHED → ERROR`. Button actions transition states, and the LCD reflects the current state.

---

## System Flow
1. **`setup()`**
   - Serial init, LCD init, button init, relay init, LED pin, RTC init, sensor init, PID init.
   - Displays "Oven Ready".
2. **`loop()`** (executed continuously)
   - **Input handling** – `setInputFlags()` → `resolveInputFlags()` → calls `inputAction()` based on button pressed.
   - **Sensor read** – humidity via `DHT22Sensor`, temperature via `DS18B20Sensor`.
   - **Safety check** – if temperature > 130 °C → `stopOven()`, state = `ERROR`.
   - **PID** – temperature updates `CURRENT_TEMPERATURE`; `computePID()` calculates heater PWM and toggles relay.
   - **RTC** – `dsHandleRTC()` advances alarms, syncs time every minute.
   - **UI** – `renderLCD()` draws the appropriate screen for the current state.
   - Small `delay(10)` to ease CPU load.

**Simplified flow diagram**
```
setup() ──► init all peripherals
   │
   ▼
loop()
 ├─► read buttons → inputAction()
 ├─► read humidity & temperature
 ├─► safety cut‑off?
 │   └─► stopOven() & ERROR state
 ├─► PID compute → heater relay
 ├─► dsHandleRTC() (alarms & sync)
 └─► renderLCD()
```

---

## User Interface (Buttons + LCD)
| Button | Function |
|--------|----------|
| **UP** (pin 4) | Increment set‑point / timer |
| **DOWN** (pin 5) | Decrement set‑point / timer |
| **SELECT** (pin 6) | Advance to next screen / confirm action / start / stop / reset |

**Screens**
| State | LCD Content |
|-------|-------------|
| **IDLE** | Current temperature, setpoint, "Press SELECT". |
| **SET_TEMP** | "SET TEMP", current set‑point, `BTN1:+  BTN2:-  BTN3:NEXT`. |
| **SET_TIMER** | "SET TIMER", minutes, same button hints. |
| **READY** | Shows target temperature & timer, `BTN3: START`. |
| **RUNNING** | Live temperature vs. set‑point, remaining timer, `BTN3: STOP`. |
| **FINISHED** | "DONE!", "Time Complete", `BTN3: RESET`. |
| **ERROR** | "ERROR – Overtemp", `BTN3: RESET`. |

---

## Control Logic
- **Set‑point range**: 60 °C – 125 °C (clamped in `SET_TEMP`).
- **Timer**: User‑selected minutes (1 – 180 min). `Alarm.timerOnce()` schedules `ovenStopCallback()`.
- **Relay behavior**
  - **Exhaust**: Always on during `RUNNING` for safety.
  - **Heater**: Controlled by PID output (0‑255) via time‑proportional window (1 s).
- **Safety**: Over‑temperature (>130 °C) forces immediate shut‑off and enters `ERROR`.

**PID defaults** (editable in `PID_CONF.cpp`)
```cpp
double P = 4.0;
double I = 0.0;
double D = 22.0;
double TEMPERATURE_SETPOINT = 100.0; // °C
```

---

## Timer & Scheduling
- **`dsInitRTC()`**: Starts I²C, initializes the DS3231, sets time from compile‑time if battery lost, and seeds TimeAlarms.
- **`dsHandleRTC()`**: Calls `Alarm.delay(1000)` each loop to advance the alarm engine, and resynchronises the Arduino `time()` with the RTC every 60 s.
- **`ovenStartAlarm()` / `ovenStopAlarm()`**: Example callbacks (currently hard‑coded to a placeholder relay pin).
- **`ovenTimer = Alarm.timerOnce(timerMinutes*60, ovenStopCallback);`** schedules a one‑shot stop alarm.

---

## Getting Started
1. **Install Arduino IDE** (or compatible CLI).
2. **Add libraries** via Library Manager or `Sketch → Include Library → Manage Libraries`
| Library | Header |
|---------|--------|
| LiquidCrystal_I2C | `LiquidCrystal_I2C.h` |
| OneWire | `OneWire.h` |
| DallasTemperature | `DallasTemperature.h` |
| DHT sensor library | `DHT.h` |
| RTClib | `RTClib.h` |
| TimeLib | `TimeLib.h` |
| TimeAlarms | `TimeAlarms.h` |
| PID_v1 | `PID_v1.h` |
3. **Wire the hardware** (see pin table above).
4. Open `src/uno/OvenController/OvenController.ino` in the IDE, select **Arduino Uno**, and click **Upload**.
5. Power the board, open the Serial Monitor (9600 baud) to see debug output, and interact with the LCD using the three buttons.

---

## Dependencies
- **Arduino core** (`Arduino.h`)
- **LiquidCrystal_I2C** – I²C LCD driver
- **OneWire** & **DallasTemperature** – DS18B20 communication
- **DHT** – DHT22 humidity sensor
- **RTClib** – DS3231 RTC handling
- **TimeLib** – time utilities (`now()`, `setTime()`)
- **TimeAlarms** – alarm & timer scheduling
- **PID_v1** – PID control algorithm
All libraries are publicly available through the Arduino Library Manager.

---

## Future Improvements
- Full PID tuning (auto‑tune, different window sizes).
- Persistent storage of set‑points & timer via EEPROM.
- Wi‑Fi / ESP32 migration for remote monitoring & OTA updates.
- Mobile or web dashboard (MQTT, HTTP).
- Additional safety sensors (over‑current, door switch).

---

## Troubleshooting
| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| LCD stays blank or shows "LCD error" | Incorrect I²C address or wiring | Verify address (`0x27`) with an I²C scanner; check SDA/SCL connections. |
| `Alarm.freeAll()` warning in Serial | TimeAlarms bug when freeing non‑existent alarms | Ensure `ovenTimer` is valid before calling `Alarm.free()`. |
| Compile error: multiple `RTClib` versions | Duplicate library versions in sketchbook | Keep only one version of `RTClib` (prefer the latest). |
| Relays never trigger | Pins not set as outputs or missing power | Confirm `initRelays()` runs; measure voltage on relay pins. |
| Over‑temperature shutdown immediately | Sensor wiring error or faulty DS18B20 | Test sensor separately; replace if readings are nonsensical. |

---

## License
**MIT License** – feel free to use, modify, and distribute this code.

---

## Author
*Created by the "oven‑controller" project maintainers.*