// #include <Arduino.h>
// #include "display_manager.h"
// #include "waveform_generator.h"
// #include "button_handler.h"
// #include "bluetooth_manager.h"
// #include "stepper_motor.h"

// // Update timing parameters - DECREASED for faster updates
// unsigned long lastUpdateTime = 0;
// const int updateInterval = 20;  // Update interval (ms) - DECREASED from 50ms to 20ms

// #define RED_LED_PIN A9  
// #define GREEN_LED_PIN A10  

// // BPM calculation
// float currentBPM = 60.0;  // Default BPM
// const float MIN_BPM = 40.0;  // Minimum allowed BPM
// const float MAX_BPM = 240.0; // Maximum allowed BPM
// const float DEFAULT_INTERNAL_BPM = 120.0; // Default BPM for internal source

// // BPM calculation variables
// const int BPM_BUFFER_SIZE = 5;  // Size of buffer for averaging BPM values
// float bpmBuffer[BPM_BUFFER_SIZE]; // Buffer to store recent BPM values
// int bpmBufferIndex = 0;  // Current index in the buffer
// bool bpmBufferFilled = false;  // Flag to indicate if buffer is filled
// unsigned long lastBpmUpdateTime = 0;
// const int bpmUpdateInterval = 250;  // Update BPM every 250ms to avoid excessive calculation

// // Function to display data source change notification
// void displayDataSourceChangeNotification(DataSource source) {
//   // Display notification on both screens
//   display1.clearDisplay();
//   display1.setTextSize(1);
//   display1.setTextColor(SSD1306_WHITE);
//   display1.setCursor(0, 0);
//   display1.println(F("Data source changed to:"));
//   display1.setTextSize(2);
//   display1.setCursor(10, 20);
//   display1.println(source == SOURCE_INTERNAL ? F("INTERNAL") : F("BLUETOOTH"));
//   display1.display();
  
//   display2.clearDisplay();
//   display2.setTextSize(1);
//   display2.setTextColor(SSD1306_WHITE);
//   display2.setCursor(0, 0);
//   display2.println(F("New data source:"));
//   display2.setTextSize(2);
//   display2.setCursor(10, 20);
//   display2.println(source == SOURCE_INTERNAL ? F("INTERNAL") : F("BLUETOOTH"));
//   display2.display();
  
//   // When switching to internal source, reset BPM to default
//   if (source == SOURCE_INTERNAL) {
//     currentBPM = DEFAULT_INTERNAL_BPM;
//     Serial.print("Switched to internal source, BPM set to: ");
//     Serial.println(currentBPM);
    
//     // Reset BPM buffer with the default value
//     for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
//       bpmBuffer[i] = currentBPM;
//     }
//   }
  
//   // Show for 2 seconds
//   delay(2000);
// }

// // Calculate BPM from Bluetooth data
// float calculateBPM(float btValue) {
//   // Map the absolute value to a BPM range
//   // Assuming btValue is normalized between -1.0 and 1.0
//   float mappedBPM = map(abs(btValue) * 100, 0, 100, MIN_BPM, MAX_BPM);
  
//   // Constrain to valid range
//   if (mappedBPM < MIN_BPM) mappedBPM = MIN_BPM;
//   if (mappedBPM > MAX_BPM) mappedBPM = MAX_BPM;
  
//   // Add to buffer for smoothing
//   bpmBuffer[bpmBufferIndex] = mappedBPM;
//   bpmBufferIndex = (bpmBufferIndex + 1) % BPM_BUFFER_SIZE;
  
//   if (bpmBufferIndex == 0) {
//     bpmBufferFilled = true;
//   }
  
//   // Calculate average BPM from buffer
//   float totalBPM = 0.0;
//   int samples = bpmBufferFilled ? BPM_BUFFER_SIZE : bpmBufferIndex;
  
//   for (int i = 0; i < samples; i++) {
//     totalBPM += bpmBuffer[i];
//   }
  
//   return (samples > 0) ? (totalBPM / samples) : MIN_BPM;
// }

// void setup() {
//   Serial.begin(115200);
//   Serial.println("Starting ESP32 Waveform Display & Metronome...");
  
//   // Initialize LED pins
//   pinMode(RED_LED_PIN, OUTPUT);
//   pinMode(GREEN_LED_PIN, OUTPUT);

//   digitalWrite(RED_LED_PIN, LOW);
//   digitalWrite(GREEN_LED_PIN, LOW);
  
//   // Initialize buffer with default BPM
//   for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
//     bpmBuffer[i] = DEFAULT_INTERNAL_BPM;  // Start with internal default BPM
//   }
  
//   // Set initial BPM
//   currentBPM = DEFAULT_INTERNAL_BPM;
  
//   // Initialize button
//   initButton();
  
//   // Initialize displays
//   if (!initDisplays()) {
//     Serial.println("Display initialization failed");
//     for(;;); // Infinite loop
//   }
  
//   // Display startup information
//   displayStartupScreen();
  
//   // Initialize waveform
//   initWaveform();
  
//   // Set waveform speed to a faster value (1-10 scale)
//   setWaveformSpeed(5);  // Set to a middle value, can be adjusted
  
//   // Initialize metronome
//   initMetronome();
  
//   // Initialize Bluetooth
//   if (!initBluetooth()) {
//     Serial.println("Bluetooth initialization failed");
//     // Continue anyway, we can still use internal data source
//   }
  
//   // Wait 2 seconds before starting
//   delay(2000);
  
//   Serial.println("Starting waveform generation and metronome...");
// }

// void loop() {
//   // Update Bluetooth status
//   updateBluetooth();
  
//   // Check for Bluetooth events
//   BluetoothEvent btEvent = checkBluetoothEvent();
//   if (btEvent == BT_EVENT_CONNECTED || btEvent == BT_EVENT_DATA_RECEIVED) {
//     digitalWrite(GREEN_LED_PIN, HIGH);  // Turn on green LED
//     digitalWrite(RED_LED_PIN, LOW);     // Turn off red LED
//   } else {
//     digitalWrite(RED_LED_PIN, HIGH);    // Turn on red LED
//     digitalWrite(GREEN_LED_PIN, LOW);   // Turn off green LED
//   }
  
//   if (btEvent == BT_EVENT_CONNECTED || btEvent == BT_EVENT_DISCONNECTED) {
//     // Display Bluetooth connection status for a brief moment
//     display1.clearDisplay();
//     display1.setTextSize(1);
//     display1.setTextColor(SSD1306_WHITE);
//     display1.setCursor(0, 0);
//     display1.println(F("Bluetooth status:"));
//     display1.setTextSize(1);
//     display1.setCursor(0, 20);
//     display1.println(getBluetoothStatusMessage());
//     display1.display();
    
//     // Short delay to show the message
//     delay(1000);
//   }
  
//   // Get current data source
//   DataSource currentSource = getDataSource();
  
//   // Update BPM based on source
//   unsigned long currentTime = millis();
//   if (currentSource == SOURCE_BLUETOOTH && isBluetoothConnected()) {
//     // Calculate BPM from Bluetooth data periodically
//     if (currentTime - lastBpmUpdateTime > bpmUpdateInterval) {
//       lastBpmUpdateTime = currentTime;
      
//       // Get Bluetooth data and calculate BPM
//       float btValue = getBluetoothWaveformValue();
//       float newBPM = calculateBPM(btValue);
      
//       // Only update if there's a significant change
//       if (abs(newBPM - currentBPM) > 3.0) {
//         currentBPM = newBPM;
//         Serial.print("BPM updated to: ");
//         Serial.println(currentBPM);
//       }
//     }
//   }
//   else if (currentSource == SOURCE_INTERNAL && currentBPM < DEFAULT_INTERNAL_BPM) {
//     // Ensure internal source uses the default BPM
//     currentBPM = DEFAULT_INTERNAL_BPM;
//   }
  
//   // Update metronome with current BPM
//   updateMetronome(currentBPM);
  
//   // Check button status
//   if (checkButton()) {
//     // Get the current data source after potential change
//     DataSource newSource = getDataSource();
    
//     // If data source was changed, show notification
//     if (newSource == SOURCE_BLUETOOTH && !isBluetoothConnected()) {
//       // Show warning if Bluetooth is selected but not connected
//       display1.clearDisplay();
//       display1.setTextSize(1);
//       display1.setTextColor(SSD1306_WHITE);
//       display1.setCursor(0, 0);
//       display1.println(F("Warning:"));
//       display1.println(F("Bluetooth source"));
//       display1.println(F("selected but not"));
//       display1.println(F("connected!"));
//       display1.display();
      
//       delay(2000);
//     } else {
//       // If source change, show data source notification
//       if (currentSource != newSource) {
//         displayDataSourceChangeNotification(newSource);
//       } else {
//         // Otherwise, this was a style change
//         displayStyleChangeNotification(getCurrentStyleName());
//       }
//     }
//   }
  
//   // Generate waveform data based on current source
//   if (currentSource == SOURCE_INTERNAL || !isBluetoothConnected()) {
//     // Use internal random waveform if:
//     // 1. Internal source is selected, or
//     // 2. Bluetooth source is selected but not connected
//     generateRandomWaveform();
//   } else {
//     // Use Bluetooth data
//     generateBluetoothWaveform(getBluetoothWaveformValue());
//   }
  
//   // Periodic display update
//   if (currentTime - lastUpdateTime > updateInterval) {
//     lastUpdateTime = currentTime;
    
//     // Call the display update function
//     updateDisplay(currentBPM);
    
//     // Add BPM display to the second screen
//     // display2.setTextSize(2);  // Larger font for BPM
//     // display2.setTextColor(SSD1306_WHITE);
//     // display2.setCursor(2, 2);
//     // display2.print(F("BPM: "));
//     // display2.print((int)currentBPM);  // Display as integer for cleaner look
    
//     // // Now update the second display after adding BPM
//     // display2.display();
//   }
// }

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include "display_manager.h"
#include "waveform_generator.h"
#include "button_handler.h"
#include "stepper_motor.h"

// BLE settings
#define SERVER_NAME "ESP32_Beat_Server"
#define SERVICE_UUID "0cebcba9-4624-4999-9cec-5debf01d8ecb"
#define CHARACTERISTIC_UUID "65f04fbf-01ac-4954-a999-af6b77c65fdf"

// LED indicator pins
#define RED_LED_PIN A9  
#define GREEN_LED_PIN A10  

// BPM calculation
float currentBPM = 60.0;  // Default BPM
const float MIN_BPM = 40.0;  // Minimum allowed BPM
const float MAX_BPM = 240.0; // Maximum allowed BPM
const float DEFAULT_INTERNAL_BPM = 120.0; // Default BPM for internal source

// BPM calculation variables
const int BPM_BUFFER_SIZE = 5;  // Size of buffer for averaging BPM values
float bpmBuffer[BPM_BUFFER_SIZE]; // Buffer to store recent BPM values
int bpmBufferIndex = 0;  // Current index in the buffer
bool bpmBufferFilled = false;  // Flag to indicate if buffer is filled
unsigned long lastBpmUpdateTime = 0;
const int bpmUpdateInterval = 250;  // Update BPM every 250ms to avoid excessive calculation

// Beat processing
unsigned long lastBeatTime = 0;
unsigned long beatInterval = 500; // Default interval between beats (ms)
float lastBeatStrength = 0.0;
bool beatReceived = false;

// BLE connection variables
BLEClient* pClient = nullptr;
BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;
bool deviceConnected = false;
bool doScan = true;
BLEAddress* serverAddress = nullptr;

// Update timing parameters
unsigned long lastUpdateTime = 0;
const int updateInterval = 50;  // Update interval (ms)

// Function to handle beat data from BLE callback
void beatNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (length >= 5 && pData[0] == 0x01) {
    // This is a beat notification
    // Extract beat strength (float value starting at index 1)
    float beatStrength;
    memcpy(&beatStrength, &pData[1], sizeof(float));
    
    // Record beat time and calculate interval
    unsigned long currentTime = millis();
    unsigned long newBeatInterval = currentTime - lastBeatTime;
    lastBeatTime = currentTime;
    
    // Only use reasonable intervals for BPM calculation
    if (newBeatInterval > 200 && newBeatInterval < 2000) {
      // Convert interval to BPM
      float newBPM = 60000.0 / newBeatInterval;
      
      // Add to BPM buffer
      bpmBuffer[bpmBufferIndex] = newBPM;
      bpmBufferIndex = (bpmBufferIndex + 1) % BPM_BUFFER_SIZE;
      
      if (bpmBufferIndex == 0) {
        bpmBufferFilled = true;
      }
      
      // Calculate average BPM
      float totalBPM = 0.0;
      int samples = bpmBufferFilled ? BPM_BUFFER_SIZE : bpmBufferIndex;
      
      for (int i = 0; i < samples; i++) {
        totalBPM += bpmBuffer[i];
      }
      
      // Update current BPM
      if (samples > 0) {
        currentBPM = totalBPM / samples;
        // Constrain to valid range
        if (currentBPM < MIN_BPM) currentBPM = MIN_BPM;
        if (currentBPM > MAX_BPM) currentBPM = MAX_BPM;
      }
    }
    
    // Store beat strength for waveform generation
    lastBeatStrength = beatStrength;
    beatReceived = true;
    
    Serial.print("Beat detected! Strength: ");
    Serial.print(beatStrength);
    Serial.print(", BPM: ");
    Serial.println(currentBPM);
  }
  else if (length >= 2) {
    // This is a waveform sample
    // Could be used for visualization if needed
    int16_t sample;
    memcpy(&sample, pData, sizeof(int16_t));
    
    // Process the sample for waveform visualization
    // Scale the sample to fit our waveform generator
    float scaledValue = sample / 32768.0; // Normalize to range -1.0 to 1.0
    generateBluetoothWaveform(scaledValue);
  }
}

// Scan for the server
class AdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE device found: ");
    Serial.println(advertisedDevice.toString().c_str());
    
    // Check if we found the server
    if (advertisedDevice.getName() == SERVER_NAME) {
      Serial.print("Found server: ");
      Serial.println(advertisedDevice.getAddress().toString().c_str());
      
      // Stop scanning and save the server address
      BLEDevice::getScan()->stop();
      serverAddress = new BLEAddress(advertisedDevice.getAddress());
      doScan = false;
    }
  }
};

// Connect to the server
bool connectToServer() {
  Serial.print("Connecting to server: ");
  Serial.println(serverAddress->toString().c_str());
  
  // Create a client
  pClient = BLEDevice::createClient();
  
  // Connect to the server
  if (!pClient->connect(*serverAddress)) {
    Serial.println("Connection failed");
    return false;
  }
  
  Serial.println("Connected to server");
  
  // Get the service
  BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.println("Failed to find service UUID");
    pClient->disconnect();
    return false;
  }
  
  // Get the characteristic
  pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.println("Failed to find characteristic UUID");
    pClient->disconnect();
    return false;
  }
  
  // Subscribe to notifications
  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(beatNotifyCallback);
    Serial.println("Registered for notifications");
  }
  
  deviceConnected = true;
  return true;
}

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

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 Beat Detection Client");
  
  // Initialize LED pins
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, HIGH);  // Start with red LED on (not connected)
  digitalWrite(GREEN_LED_PIN, LOW);
  
  // Initialize buffer with default BPM
  for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
    bpmBuffer[i] = DEFAULT_INTERNAL_BPM;
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
  
  // Initialize metronome
  initMetronome();
  
  // Initialize BLE
  BLEDevice::init("");
  
  // Create a scan
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  
  // Start scanning for servers
  Serial.println("Scanning for beat detection server...");
  pBLEScan->start(10, false);
  
  // Wait 2 seconds before starting
  delay(2000);
}

void loop() {
  // Handle scanning and connection
  if (doScan) {
    BLEDevice::getScan()->start(5, false);
    delay(500);
  } else if (serverAddress != nullptr && !deviceConnected) {
    // Try to connect to the server
    if (connectToServer()) {
      digitalWrite(GREEN_LED_PIN, HIGH);  // Turn on green LED
      digitalWrite(RED_LED_PIN, LOW);     // Turn off red LED
      
      // Switch to Bluetooth data source
      setDataSource(SOURCE_BLUETOOTH);
      displayDataSourceChangeNotification(SOURCE_BLUETOOTH);
    } else {
      // Scan again if connection failed
      doScan = true;
      digitalWrite(RED_LED_PIN, HIGH);    // Turn on red LED
      digitalWrite(GREEN_LED_PIN, LOW);   // Turn off green LED
    }
  }
  
  // If connected but lost connection
  if (!deviceConnected && pClient != nullptr) {
    // Clean up
    delete pClient;
    pClient = nullptr;
    pRemoteCharacteristic = nullptr;
    
    // Start scanning again
    doScan = true;
    
    // Update LED indicators
    digitalWrite(RED_LED_PIN, HIGH);    // Turn on red LED
    digitalWrite(GREEN_LED_PIN, LOW);   // Turn off green LED
    
    // Switch to internal data source
    setDataSource(SOURCE_INTERNAL);
    displayDataSourceChangeNotification(SOURCE_INTERNAL);
  }
  
  // Get current data source
  DataSource currentSource = getDataSource();
  
  // Check button status
  if (checkButton()) {
    // Get the current data source after potential change
    DataSource newSource = getDataSource();
    
    // If data source was changed, show notification
    if (newSource != currentSource) {
      displayDataSourceChangeNotification(newSource);
      
      // If changed to internal, reset BPM
      if (newSource == SOURCE_INTERNAL) {
        currentBPM = DEFAULT_INTERNAL_BPM;
      }
    } else {
      // Otherwise, this was a style change
      displayStyleChangeNotification(getCurrentStyleName());
    }
  }
  
  // Generate waveform data based on current source
  if (currentSource == SOURCE_INTERNAL) {
    // Use internal random waveform
    generateRandomWaveform();
  } else if (!deviceConnected) {
    // If Bluetooth source is selected but not connected, fallback to internal
    generateRandomWaveform();
  }
  // Note: Bluetooth waveform is generated in the callback
  
  // Update metronome with current BPM
  updateMetronome(currentBPM);
  
  // Periodic display update
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime > updateInterval) {
    lastUpdateTime = currentTime;
    updateDisplay();
    
    // Add BPM display to the screen
    display2.setCursor(2, SCREEN_HEIGHT - 20);
    display2.print(F("BPM: "));
    display2.print(currentBPM, 1);
    
    // Show beat indicator if recent beat
    if (currentTime - lastBeatTime < 200) {
      display1.fillCircle(SCREEN_WIDTH - 10, 10, 5, SSD1306_WHITE);
    }
    
    display1.display();
    display2.display();
  }
  
  // Small delay to avoid overwhelming the CPU
  delay(5);
}