// LCD configuration implementation for LiquidCrystal_I2C display.
#include "LCD_CONF.h"

// Define the LCD instance with I2C address 0x27 and 20x4 dimensions.
LiquidCrystal_I2C lcd(0x27, 20, 4);

void initLCD() {
    lcd.init();
    lcd.backlight();
}

void clearLCD() {
    lcd.clear();
}

void setLCDText(const String &text, int x, int y) {
    lcd.setCursor(x, y);
    lcd.print(text);
}

void setLCDText(double value, int x, int y) {
    lcd.setCursor(x, y);
    lcd.print(value);
}

void setLCDText(float value, int x, int y) {
    lcd.setCursor(x, y);
    lcd.print(value);
}

void setLCDText(int value, int x, int y) {
    lcd.setCursor(x, y);
    lcd.print(value);
}

void setLCDText(char value, int x, int y) {
    lcd.setCursor(x, y);
    lcd.print(value);
}