#include "button_handler.h"
#include "waveform_generator.h"

// Current display style
static DisplayStyle currentStyle = STYLE_NORMAL;

// Button state variables
static int buttonState = HIGH;              // Current button state
static int lastButtonState = HIGH;          // Previous button state
static unsigned long buttonPressTime = 0;   // Time when button was pressed
static unsigned long longPressTime = 1000;  // Long press duration (ms)
static bool longPressDetected = false;      // Flag for long press detection

void initButton() {
  // Set button pin as input with pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize button state
  buttonState = digitalRead(BUTTON_PIN);
  lastButtonState = buttonState;

  // Initialize display style
  currentStyle = STYLE_NORMAL;

  Serial.println("Button handler initialized");
}

bool checkButton() {
  bool styleChanged = false;
  bool sourceChanged = false;

  // Read current button state
  buttonState = digitalRead(BUTTON_PIN);

  // // Button press detected (from HIGH to LOW)
  // if (lastButtonState == HIGH && buttonState == LOW) {
  //   // Record the time when button was pressed
  //   buttonPressTime = millis();
  //   // Reset long press detection flag
  //   longPressDetected = false;

  //   // Debug output
  //   Serial.println("Button pressed");
  // }
  // // Button release detected (from LOW to HIGH)
  // else if (lastButtonState == LOW && buttonState == HIGH) {
  //   // Calculate how long the button was pressed
  //   unsigned long pressDuration = millis() - buttonPressTime;
  //   Serial.println(longPressDetected);
  //   // Handle long press (change data source)
  //   if (pressDuration >= longPressTime && !longPressDetected) {
  //     // Toggle between data sources
  //     DataSource currentSource = getDataSource();
  //     DataSource newSource = (currentSource == SOURCE_INTERNAL) ? SOURCE_BLUETOOTH : SOURCE_INTERNAL;
  //     setDataSource(newSource);

  //     // Debug output for data source change
  //     Serial.print("Long press detected! Data source changed to: ");
  //     Serial.println((newSource == SOURCE_INTERNAL) ? "Internal" : "Bluetooth");

  //     sourceChanged = true;
  //   }
  //   // Handle short press (change display style)
  //   else if (pressDuration < longPressTime && !longPressDetected) {
  //     // Switch to next display style
  //     currentStyle = (DisplayStyle)((currentStyle + 1) % STYLE_COUNT);

  //     // Debug output for style change
  //     Serial.print("Short press! Style changed to: ");
  //     Serial.println(getCurrentStyleName());

  //     styleChanged = true;
  //   }

  //   // Debug duration
  //   Serial.print("Button press duration: ");
  //   Serial.print(pressDuration);
  //   Serial.println(" ms");
  // }
  // // While button is held down, check for long press
  // else if (buttonState == LOW) {
  //   // Check if it's been held long enough for a long press
  //   if ((millis() - buttonPressTime) >= longPressTime && !longPressDetected) {
  //     // Set flag to prevent multiple triggers
  //     longPressDetected = true;

  //     // Visual feedback of long press detection could be added here
  //     Serial.println("Long press detected while button is still held");
  //   }
  // }
  // When button is first pressed (transition from HIGH to LOW)
  if (lastButtonState == HIGH && buttonState == LOW) {
    // Record the time when button was pressed
    buttonPressTime = millis();
    longPressDetected = false;  // Reset the long press flag at the start of new press

    Serial.println("Button pressed");
  }
  // When button is released (transition from LOW to HIGH)
  else if (lastButtonState == LOW && buttonState == HIGH) {
    // Calculate how long the button was pressed
    unsigned long pressDuration = millis() - buttonPressTime;

    // Handle based on press duration
    if (pressDuration >= longPressTime) {
      // Handle long press (change data source)
      DataSource currentSource = getDataSource();
      DataSource newSource = (currentSource == SOURCE_INTERNAL) ? SOURCE_BLUETOOTH : SOURCE_INTERNAL;
      setDataSource(newSource);

      // Debug output for data source change
      Serial.print("Long press action! Data source changed to: ");
      Serial.println((newSource == SOURCE_INTERNAL) ? "Internal" : "Bluetooth");

      sourceChanged = true;
    } else {
      // Handle short press (change display style)
      currentStyle = (DisplayStyle)((currentStyle + 1) % STYLE_COUNT);

      // Debug output for style change
      Serial.print("Short press action! Style changed to: ");
      Serial.println(getCurrentStyleName());

      styleChanged = true;
    }

    // Debug duration
    Serial.print("Button press duration: ");
    Serial.print(pressDuration);
    Serial.println(" ms");

    // Reset long press detection for next press
    longPressDetected = false;
  }
  // While button is held down, provide feedback for long press if needed
  else if (buttonState == LOW) {
    // Check if it's been held long enough for a long press
    unsigned long currentPressDuration = millis() - buttonPressTime;

    // Detect exactly when we cross the threshold (only trigger once)
    if (currentPressDuration >= longPressTime && !longPressDetected) {
      // Set flag to prevent multiple triggers
      longPressDetected = true;

      // Visual feedback of long press detection could be added here
      Serial.println("Long press threshold reached - release to execute");

      // Optional: You could provide visual feedback here to indicate
      // that a long press has been detected (e.g., blink LED)
    }
  }
  // Save current button state for next comparison
  lastButtonState = buttonState;

  // Return true if either style or source changed
  return styleChanged || sourceChanged;
}

int getButtonState() {
  return buttonState;
}

DisplayStyle getCurrentStyle() {
  return currentStyle;
}

const char* getCurrentStyleName() {
  // Return string description of current style
  switch (currentStyle) {
    case STYLE_NORMAL:
      return "Line";
    case STYLE_FILLED:
      return "Filled";
    case STYLE_DOTS:
      return "Dots";
    case STYLE_BAR:
      return "Bars";
    default:
      return "Unknown";
  }
}