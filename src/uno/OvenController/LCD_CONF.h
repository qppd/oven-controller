// LCD configuration header for LiquidCrystal_I2C display.
#ifndef LCD_CONF_H
#define LCD_CONF_H

#include <LiquidCrystal_I2C.h>

// Extern declaration of the LCD instance.
extern LiquidCrystal_I2C lcd;

// Initialize the LCD and turn on backlight.
void initLCD();

// Clear the LCD display.
void clearLCD();

// Set text on the LCD at position (x, y). Overloads for various data types.
void setLCDText(const String &text, int x, int y);
void setLCDText(double value, int x, int y);
void setLCDText(float value, int x, int y);
void setLCDText(int value, int x, int y);
void setLCDText(char value, int x, int y);

#endif // LCD_CONF_H