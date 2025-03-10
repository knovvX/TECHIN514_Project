#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Screen parameters
#define SCREEN_WIDTH 128     // OLED display width in pixels
#define SCREEN_HEIGHT 64     // OLED display height in pixels
#define OLED_RESET -1        // Reset pin, -1 means Arduino reset pin
#define SCREEN1_ADDRESS 0x3D // First screen address
#define SCREEN2_ADDRESS 0x3C // Second screen address

// External declaration of display objects for use in other modules
extern Adafruit_SSD1306 display1;
extern Adafruit_SSD1306 display2;

// Initialize displays
bool initDisplays();

// Display startup screen
void displayStartupScreen();

// Update display content
void updateDisplay(float bpm);
// Updated display function that accepts BPM
// void updateDisplayWithBPM(float bpm);

// Display style change notification
void displayStyleChangeNotification(const char* styleName);

// Different styles of waveform drawing functions
void drawLineWaveform(int screen);
void drawFilledWaveform(int screen);
void drawDotWaveform(int screen);
void drawBarWaveform(int screen);

#endif // DISPLAY_MANAGER_H