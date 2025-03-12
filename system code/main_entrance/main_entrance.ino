#include <Arduino.h>
#include "display_manager.h"
#include "waveform_generator.h"
#include "button_handler.h"
#include "bluetooth_manager.h"
#include "stepper_motor.h"

// Update timing parameters - DECREASED for faster updates
unsigned long lastUpdateTime = 0;
const int updateInterval = 20;  // Update interval (ms) - DECREASED from 50ms to 20ms

#define RED_LED_PIN A10  
#define GREEN_LED_PIN A9  
#define GREEN_LED_BRIGHTNESS 32 
// BPM calculation
float currentBPM = 90.0;  // Default BPM
const float MIN_BPM = 40.0;  // Minimum allowed BPM
const float MAX_BPM = 220.0; // Maximum allowed BPM
const float DEFAULT_INTERNAL_BPM = 120.0; // Default BPM for internal source

// BPM calculation variables
const int BPM_BUFFER_SIZE = 5;  // Size of buffer for averaging BPM values
float bpmBuffer[BPM_BUFFER_SIZE]; // Buffer to store recent BPM values
int bpmBufferIndex = 0;  // Current index in the buffer
bool bpmBufferFilled = false;  // Flag to indicate if buffer is filled
unsigned long lastBpmUpdateTime = 0;
const int bpmUpdateInterval = 250;  // Update BPM every 250ms to avoid excessive calculation

// Function to display data source change notification
void displayDataSourceChangeNotification(DataSource source) {
  // Display notification on both screens
  display1.clearDisplay();
  display1.setTextSize(1);
  display1.setTextColor(SSD1306_WHITE);
  display1.setCursor(0, 0);
  display1.println(F("Data source changed to:"));
  display1.setTextSize(2);
  display1.setCursor(10, 20);
  display1.println(source == SOURCE_INTERNAL ? F("INTERNAL") : F("BLUETOOTH"));
  display1.display();
  
  display2.clearDisplay();
  display2.setTextSize(1);
  display2.setTextColor(SSD1306_WHITE);
  display2.setCursor(0, 0);
  display2.println(F("New data source:"));
  display2.setTextSize(2);
  display2.setCursor(10, 20);
  display2.println(source == SOURCE_INTERNAL ? F("INTERNAL") : F("BLUETOOTH"));
  display2.display();
  
  // When switching to internal source, reset BPM to default
  if (source == SOURCE_INTERNAL) {
    currentBPM = DEFAULT_INTERNAL_BPM;
    Serial.print("Switched to internal source, BPM set to: ");
    Serial.println(currentBPM);
    
    // Reset BPM buffer with the default value
    for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
      bpmBuffer[i] = currentBPM;
    }
  }
  
  // Show for 2 seconds
  delay(2000);
}

// Calculate BPM from Bluetooth data
// For a real BPM calculation:
// float calculateBPM() {
//   // Time interval for measuring beats (in milliseconds)
//   const unsigned long measurementWindow = 5000; // 5 seconds sliding window
  
//   // Store timestamps of detected beats
//   static const int MAX_BEATS = 30;
//   static unsigned long beatTimes[MAX_BEATS];
//   static int beatCount = 0;
//   static int oldestBeatIndex = 0;
  
//   // Current time
//   unsigned long currentTime = millis();
  
//   // Record this beat time and increment count
//   beatTimes[oldestBeatIndex] = currentTime;
//   oldestBeatIndex = (oldestBeatIndex + 1) % MAX_BEATS;
//   if (beatCount < MAX_BEATS) beatCount++;
  
//   // Calculate BPM based on recorded beats
//   if (beatCount < 2) return MIN_BPM; // Need at least 2 beats
  
//   // Find oldest beat that's within our measurement window
//   int validBeatCount = 0;
//   int oldestValidBeat = oldestBeatIndex;
  
//   for (int i = 0; i < beatCount; i++) {
//     int index = (oldestBeatIndex - i + MAX_BEATS) % MAX_BEATS;
//     if (currentTime - beatTimes[index] <= measurementWindow) {
//       validBeatCount++;
//       oldestValidBeat = index;
//     } else {
//       break; // Found an old beat outside our window
//     }
//   }
  
//   if (validBeatCount < 2) return MIN_BPM; // Need at least 2 valid beats
  
//   // Calculate time span between oldest valid beat and now
//   unsigned long timeSpan = currentTime - beatTimes[oldestValidBeat];
  
//   // Convert to BPM: (beats-1) / timeSpan in minutes
//   // We use beats-1 because we're measuring intervals between beats
//   float bpm = (validBeatCount - 1) * 60000.0 / timeSpan;
  
//   // Constrain to valid range
//   if (bpm < MIN_BPM) bpm = MIN_BPM;
//   if (bpm > MAX_BPM) bpm = MAX_BPM;
  
//   return bpm;
// }

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 Waveform Display & Metronome...");
  
  // Initialize LED pins
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  
  
  // Set initial state (off)
  ledcWrite(0, 0);
  
  // Initialize buffer with default BPM
  for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
    bpmBuffer[i] = DEFAULT_INTERNAL_BPM;  // Start with internal default BPM
  }
  
  // Set initial BPM
  currentBPM = DEFAULT_INTERNAL_BPM;
  
  // Initialize button
  initButton();
  
  // Initialize displays
  if (!initDisplays()) {
    Serial.println("Display initialization failed");
    for(;;); // Infinite loop
  }
  
  // Display startup information
  displayStartupScreen();
  
  // Initialize waveform
  initWaveform();
  
  // Set waveform speed to a faster value (1-10 scale)
  setWaveformSpeed(5);  // Set to a middle value, can be adjusted
  
  // Initialize metronome
  initMetronome();
  
  // Initialize Bluetooth
  if (!initBluetooth()) {
    Serial.println("Bluetooth initialization failed");
    // Continue anyway, we can still use internal data source
  }
  
  // Wait 2 seconds before starting
  delay(2000);
  
  Serial.println("Starting waveform generation and metronome...");
}

void loop() {
  // Update Bluetooth status
  updateBluetooth();
  
  // Check for Bluetooth events
  BluetoothEvent btEvent = checkBluetoothEvent();
  if (btEvent == BT_EVENT_CONNECTED || btEvent == BT_EVENT_DATA_RECEIVED) {
    // digitalWrite(GREEN_LED_PIN, HIGH);  // Turn on green LED
    analogWrite(GREEN_LED_PIN, GREEN_LED_BRIGHTNESS);
    digitalWrite(RED_LED_PIN, LOW);     // Turn off red LED
  } else {
    digitalWrite(RED_LED_PIN, HIGH);    // Turn on red LED
    // digitalWrite(GREEN_LED_PIN, LOW);   // Turn off green LED
     analogWrite(GREEN_LED_PIN, 0);
    // ledcWrite(0, 0);
  }
  
  if (btEvent == BT_EVENT_CONNECTED || btEvent == BT_EVENT_DISCONNECTED) {
    // Display Bluetooth connection status for a brief moment
    display1.clearDisplay();
    display1.setTextSize(1);
    display1.setTextColor(SSD1306_WHITE);
    display1.setCursor(0, 0);
    display1.println(F("Bluetooth status:"));
    display1.setTextSize(1);
    display1.setCursor(0, 20);
    display1.println(getBluetoothStatusMessage());
    display1.display();
    
    // Short delay to show the message
    delay(1000);
  }
  
  // Get current data source
  DataSource currentSource = getDataSource();
  
  // Update BPM based on source
  unsigned long currentTime = millis();
  if (currentSource == SOURCE_BLUETOOTH && isBluetoothConnected()) {
    // Calculate BPM from Bluetooth data periodically
    // Serial.println(currentTime);
    // Serial.println(lastBpmUpdateTime);
    if (currentTime - lastBpmUpdateTime > bpmUpdateInterval) {
      lastBpmUpdateTime = currentTime;
      Serial.println("Yes");
      // Get Bluetooth data and calculate BPM
      // float btValue = getBluetoothWaveformValue();
      // float newBPM = calculateBPM(); 
       float newBPM = calculateBPMFromBeats();
      
      // Only update if there's a significant change
      if (abs(newBPM - currentBPM) > 3.0) {
        currentBPM = newBPM;
        Serial.print("BPM updated to: ");
        Serial.println(currentBPM);
      }
    }
  }
  else if (currentSource == SOURCE_INTERNAL && currentBPM < DEFAULT_INTERNAL_BPM) {
    // Ensure internal source uses the default BPM
    currentBPM = DEFAULT_INTERNAL_BPM;
  }
  
  // Update metronome with current BPM
  updateMetronome(currentBPM);
  
  // Check button status
  if (checkButton()) {
    // Get the current data source after potential change
    DataSource newSource = getDataSource();
    
    // If data source was changed, show notification
    if (newSource == SOURCE_BLUETOOTH && !isBluetoothConnected()) {
      // Show warning if Bluetooth is selected but not connected
      display1.clearDisplay();
      display1.setTextSize(1);
      display1.setTextColor(SSD1306_WHITE);
      display1.setCursor(0, 0);
      display1.println(F("Warning:"));
      display1.println(F("Bluetooth source"));
      display1.println(F("selected but not"));
      display1.println(F("connected!"));
      display1.display();
      
      delay(2000);
    } else {
      // If source change, show data source notification
      if (currentSource != newSource) {
        displayDataSourceChangeNotification(newSource);
      } else {
        // Otherwise, this was a style change
        displayStyleChangeNotification(getCurrentStyleName());
      }
    }
  }
  
  // Generate waveform data based on current source
  if (currentSource == SOURCE_INTERNAL || !isBluetoothConnected()) {
    // Use internal random waveform if:
    // 1. Internal source is selected, or
    // 2. Bluetooth source is selected but not connected
    generateRandomWaveform();
  } else {
    // Use Bluetooth data
    generateBluetoothWaveform(getBluetoothWaveformValue());
  }
  
  // Periodic display update
  if (currentTime - lastUpdateTime > updateInterval) {
    lastUpdateTime = currentTime;
    
    // Call the display update function
    updateDisplay(currentBPM);
    
    // Add BPM display to the second screen
    // display2.setTextSize(2);  // Larger font for BPM
    // display2.setTextColor(SSD1306_WHITE);
    // display2.setCursor(2, 2);
    // display2.print(F("BPM: "));
    // display2.print((int)currentBPM);  // Display as integer for cleaner look
    
    // // Now update the second display after adding BPM
    // display2.display();
  }
}

