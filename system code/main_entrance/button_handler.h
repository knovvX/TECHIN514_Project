#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// Button pin definition
#define BUTTON_PIN A8  // Button connected to GPIO4

// Style definitions
#define STYLE_COUNT 4      // Total number of display styles
enum DisplayStyle {
  STYLE_NORMAL = 0,        // Normal style - line waveform
  STYLE_FILLED = 1,        // Filled style - filled bottom of waveform
  STYLE_DOTS = 2,          // Dot style - only data points
  STYLE_BAR = 3            // Bar style - vertical bars
};

// Initialize button
void initButton();

// Check button status
// Returns: true if button action was detected (style or source changed), false if no change
bool checkButton();

// Get current button state
int getButtonState();

// Get current display style
DisplayStyle getCurrentStyle();

// Get current style name
const char* getCurrentStyleName();

#endif // BUTTON_HANDLER_H