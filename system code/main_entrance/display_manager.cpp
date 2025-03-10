#include "display_manager.h"
#include "button_handler.h"
#include "bluetooth_manager.h"
#include "waveform_generator.h"  

// Create two screen objects
Adafruit_SSD1306 display1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool initDisplays() {
  // Initialize I2C bus
  Wire.begin();
  
  // Initialize first screen
  if(!display1.begin(SSD1306_SWITCHCAPVCC, SCREEN1_ADDRESS)) {
    Serial.println(F("First SSD1306 initialization failed"));
    return false;
  }
  
  // Initialize second screen
  if(!display2.begin(SSD1306_SWITCHCAPVCC, SCREEN2_ADDRESS)) {
    Serial.println(F("Second SSD1306 initialization failed"));
    return false;
  }
  
  Serial.println("Both displays initialized successfully");
  return true;
}

void displayStartupScreen() {
  // Clear and display startup information
  display1.clearDisplay();
  display1.setTextSize(1);
  display1.setTextColor(SSD1306_WHITE);
  display1.setCursor(0, 0);
  display1.println(F("Waveform Display"));
  display1.println(F("With BLE"));
  display1.println(F("---------------"));
  display1.println(F("Short press: Style"));
  display1.println(F("Long press: Source"));
  display1.display();
  
  display2.clearDisplay();
  display2.setTextSize(1);
  display2.setTextColor(SSD1306_WHITE);
  display2.setCursor(0, 0);
  display2.println(F("Current style:"));
  display2.setTextSize(2);
  display2.setCursor(10, 20);
  display2.println(getCurrentStyleName());
  display2.setTextSize(1);
  display2.setCursor(0, 45);
  display2.println(F("Scanning for BLE..."));
  display2.display();
}

void updateDisplay(float bpm) {
  // Clear both displays
  display1.clearDisplay();
  display2.clearDisplay();
  
  // Get waveform parameters
  int baseHeight = getBaseHeight();
  
  // Draw baseline
  display1.drawFastHLine(0, baseHeight, SCREEN_WIDTH, SSD1306_WHITE);
  display2.drawFastHLine(0, baseHeight, SCREEN_WIDTH, SSD1306_WHITE);
  
  // Draw waveform based on current style
  DisplayStyle currentStyle = getCurrentStyle();
  switch (currentStyle) {
    case STYLE_NORMAL:
      drawLineWaveform(1);
      drawLineWaveform(2);
      break;
    case STYLE_FILLED:
      drawFilledWaveform(1);
      drawFilledWaveform(2);
      break;
    case STYLE_DOTS:
      drawDotWaveform(1);
      drawDotWaveform(2);
      break;
    case STYLE_BAR:
      drawBarWaveform(1);
      drawBarWaveform(2);
      break;
  }
  
  // Display current style name and data source
  display1.setTextSize(1);
  display1.setTextColor(SSD1306_WHITE);
  display1.setCursor(2, 2);
  display1.print(F("Style: "));
  display1.print(getCurrentStyleName());
  
  // Display data source
  display1.setCursor(2, SCREEN_HEIGHT - 10);
  display1.print(F("Source: "));
  display1.print(getDataSource() == SOURCE_INTERNAL ? "Internal" : "BLE");
  
  // Display frequency and amplitude
  // float frequency = getFrequency();
  // int amplitude = getAmplitude();
  // display2.setTextSize(1);
  // display2.setTextColor(SSD1306_WHITE);
  // display2.setCursor(2, 2);
  // display2.print(F("Freq: "));
  // display2.print(frequency, 1);
  // display2.print(F(" Amp: "));
  // display2.print(amplitude);
  display2.setTextSize(1);
  display2.setTextColor(SSD1306_WHITE);
  display2.setCursor(2, 2);
  display2.print(F("BPM: "));
  display2.print(bpm, 1);
  
  
  // Display BLE status
  display2.setCursor(2, SCREEN_HEIGHT - 10);
  display2.print(F("BLE: "));
  if (isBluetoothConnected()) {
    display2.print("Connected");
  } else {
    display2.print("Scanning...");
  }
  
  // Update displays
  display1.display();
  display2.display();
}

void displayStyleChangeNotification(const char* styleName) {
  
}

void drawLineWaveform(int screen) {
  int* waveform = getWaveformData();
  int waveformPos = getWaveformPos();
  int totalWidth = SCREEN_WIDTH * 2;
  int startIdx = 0;
  Adafruit_SSD1306* display;
  
  if (screen == 1) {
    display = &display1;
  } else {
    display = &display2;
    startIdx = SCREEN_WIDTH;
  }
  
  // Standard line waveform - connect data points
  for (int i = 0; i < SCREEN_WIDTH - 1; i++) {
    int x1 = i;
    int y1 = waveform[(waveformPos + startIdx + i) % totalWidth];
    int x2 = i + 1;
    int y2 = waveform[(waveformPos + startIdx + i + 1) % totalWidth];
    
    display->drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }
}

void drawFilledWaveform(int screen) {
  int* waveform = getWaveformData();
  int waveformPos = getWaveformPos();
  int totalWidth = SCREEN_WIDTH * 2;
  int startIdx = 0;
  int baseY = SCREEN_HEIGHT - 1;  // Bottom position
  Adafruit_SSD1306* display;
  
  if (screen == 1) {
    display = &display1;
  } else {
    display = &display2;
    startIdx = SCREEN_WIDTH;
  }
  
  // Filled waveform - fill from data point to bottom
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    int x = i;
    int y = waveform[(waveformPos + startIdx + i) % totalWidth];
    
    // Draw vertical line from data point to bottom
    display->drawLine(x, y, x, baseY, SSD1306_WHITE);
  }
}

void drawDotWaveform(int screen) {
  int* waveform = getWaveformData();
  int waveformPos = getWaveformPos();
  int totalWidth = SCREEN_WIDTH * 2;
  int startIdx = 0;
  Adafruit_SSD1306* display;
  
  if (screen == 1) {
    display = &display1;
  } else {
    display = &display2;
    startIdx = SCREEN_WIDTH;
  }
  
  // Dot waveform - only draw data points
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    int x = i;
    int y = waveform[(waveformPos + startIdx + i) % totalWidth];
    
    // Draw single pixel
    display->drawPixel(x, y, SSD1306_WHITE);
  }
}

void drawBarWaveform(int screen) {
  int* waveform = getWaveformData();
  int waveformPos = getWaveformPos();
  int totalWidth = SCREEN_WIDTH * 2;
  int startIdx = 0;
  int centerY = SCREEN_HEIGHT / 2;  // Center line position
  Adafruit_SSD1306* display;
  
  if (screen == 1) {
    display = &display1;
  } else {
    display = &display2;
    startIdx = SCREEN_WIDTH;
  }
  
  // Bar chart - vertical bars from center line to data point
  for (int i = 0; i < SCREEN_WIDTH; i += 3) { // Draw every 3 pixels for better visibility
    int x = i;
    int y = waveform[(waveformPos + startIdx + i) % totalWidth];
    
    // Draw vertical bar from center line to data point
    display->drawLine(x, centerY, x, y, SSD1306_WHITE);
    // Make bars thicker for better visual effect
    if (i < SCREEN_WIDTH - 1) {
      display->drawLine(x+1, centerY, x+1, y, SSD1306_WHITE);
    }
  }
}

// void updateDisplayWithBPM(float bpm) {
//   // Clear both displays
//   display1.clearDisplay();
//   display2.clearDisplay();
  
//   // [Rest of display code remains the same]
  
//   // Display BPM on second screen
//   display2.setTextSize(1);
//   display2.setTextColor(SSD1306_WHITE);
//   display2.setCursor(2, 2);
//   display2.print(F("BPM: "));
//   display2.print(bpm, 1);
  
//   // [Rest of display code remains the same]
  
//   // Update displays
//   display1.display();
//   display2.display();
// }