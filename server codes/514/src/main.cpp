// #include <ESP_I2S.h>
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>
// #include <Arduino.h>
// #include <math.h>

// // I2S settings
// I2SClass I2S;
// #define SAMPLE_RATE 16000
// #define PDM_DATA_PIN 41
// #define PDM_CLOCK_PIN 42

// // Buffer settings
// #define BUFFER_SIZE 512  // Larger buffer for better frequency analysis
// int16_t sampleBuffer[BUFFER_SIZE];
// int16_t processedBuffer[BUFFER_SIZE]; // Buffer for processed samples
// int bufferIndex = 0;

// // BLE server and characteristic definitions
// BLEServer *pServer = NULL;
// BLECharacteristic *pCharacteristic = NULL;
// bool deviceConnected = false;
// bool oldDeviceConnected = false;

// // Define UUIDs for the service and characteristic
// #define SERVICE_UUID "0cebcba9-4624-4999-9cec-5debf01d8ecb"
// #define CHARACTERISTIC_UUID "65f04fbf-01ac-4954-a999-af6b77c65fdf"

// // Beat detection parameters
// #define ENERGY_SMOOTHING 0.2      // Smoothing factor for energy calculation
// #define BEAT_THRESHOLD 1.5        // Energy must be this times the average to trigger a beat
// #define MIN_BEAT_INTERVAL 250     // Minimum time between beats (ms)
// #define BEAT_HOLD_TIME 100        // Hold the beat indicator for this long (ms)

// // Energy buffer for beat detection
// #define ENERGY_BUFFER_SIZE 32
// float energyBuffer[ENERGY_BUFFER_SIZE];
// int energyIndex = 0;
// float currentEnergy = 0;
// float averageEnergy = 0;
// bool beatDetected = false;
// unsigned long lastBeatTime = 0;
// unsigned long beatIndicatorTime = 0;
// float beatStrength = 0.0;

// // Noise reduction parameters
// #define NOISE_FLOOR 300            // Initial noise floor estimate
// #define NOISE_ADAPT_RATE 0.01      // Rate to adapt noise floor estimate
// float noiseFloor = NOISE_FLOOR;

// // Response variables to BLE requests
// #define MESSAGE_SIZE 20
// uint8_t messageData[MESSAGE_SIZE]; // Data to send over BLE

// // Server callbacks class to detect connection status
// class MyServerCallbacks: public BLEServerCallbacks {
//     void onConnect(BLEServer* pServer) {
//       deviceConnected = true;
//       Serial.println("Device connected");
//     }

//     void onDisconnect(BLEServer* pServer) {
//       deviceConnected = false;
//       Serial.println("Device disconnected");
//     }
// };

// // Remove DC offset and reduce noise
// void applyNoiseReduction(int16_t* samples, int16_t* output, int numSamples) {
//   // Calculate the average (DC offset)
//   int32_t sum = 0;
//   for (int i = 0; i < numSamples; i++) {
//     sum += samples[i];
//   }
//   int16_t dcOffset = sum / numSamples;
  
//   // Apply DC offset removal and noise gate
//   for (int i = 0; i < numSamples; i++) {
//     // Remove DC offset
//     output[i] = samples[i] - dcOffset;
    
//     // Apply noise gate
//     if (abs(output[i]) < noiseFloor) {
//       output[i] = 0;
//     }
//   }
  
//   // Update noise floor estimate (adapt to environment)
//   int16_t currentNoiseLevel = 0;
//   for (int i = 0; i < numSamples; i++) {
//     currentNoiseLevel += abs(output[i]);
//   }
//   currentNoiseLevel /= numSamples;
  
//   // Only adapt noise floor during quiet periods
//   if (currentNoiseLevel < noiseFloor * 2) {
//     noiseFloor = (1 - NOISE_ADAPT_RATE) * noiseFloor + NOISE_ADAPT_RATE * currentNoiseLevel;
//   }
// }

// // Detect beats using energy-based algorithm
// bool detectBeat(int16_t* samples, int numSamples) {
//   // Calculate signal energy (RMS)
//   float energy = 0;
//   for (int i = 0; i < numSamples; i++) {
//     energy += (float)samples[i] * samples[i];
//   }
//   energy = sqrt(energy / numSamples);
  
//   // Smooth the energy value
//   currentEnergy = (1 - ENERGY_SMOOTHING) * currentEnergy + ENERGY_SMOOTHING * energy;
  
//   // Store in circular buffer
//   energyBuffer[energyIndex] = currentEnergy;
//   energyIndex = (energyIndex + 1) % ENERGY_BUFFER_SIZE;
  
//   // Calculate average energy
//   float sum = 0;
//   for (int i = 0; i < ENERGY_BUFFER_SIZE; i++) {
//     sum += energyBuffer[i];
//   }
//   averageEnergy = sum / ENERGY_BUFFER_SIZE;
  
//   // Check if enough time has passed since the last beat
//   unsigned long currentTime = millis();
//   if (currentTime - lastBeatTime < MIN_BEAT_INTERVAL) {
//     return false;
//   }
  
//   // Beat detection logic
//   if (currentEnergy > averageEnergy * BEAT_THRESHOLD && currentEnergy > noiseFloor * 3) {
//     lastBeatTime = currentTime;
//     beatIndicatorTime = currentTime;
    
//     // Calculate beat strength (normalized)
//     beatStrength = constrain((currentEnergy - averageEnergy) / averageEnergy, 0.0, 1.0);
    
//     return true;
//   }
  
//   return false;
// }

// // Send beat data to client
// void sendBeatData(float strength) {
//   if (!deviceConnected) return;
  
//   // Create a structured message: [flag byte, strength float (4 bytes)]
//   messageData[0] = 0x01; // Beat indicator flag
  
//   // Convert float to bytes
//   memcpy(&messageData[1], &strength, sizeof(float));
  
//   // Send the data
//   pCharacteristic->setValue(messageData, 5); // 1 flag byte + 4 bytes float
//   pCharacteristic->notify();
  
//   Serial.print("Beat detected! Strength: ");
//   Serial.println(strength);
// }

// void setup() {
//   // Initialize serial
//   Serial.begin(115200);
//   while (!Serial) {
//     ; // wait for serial port to connect
//   }
//   Serial.println("ESP32 PDM Beat Detection Server");

//   // Setup I2S for PDM microphone
//   I2S.setPinsPdmRx(PDM_CLOCK_PIN, PDM_DATA_PIN);
  
//   if (!I2S.begin(I2S_MODE_PDM_RX, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
//     Serial.println("Failed to initialize I2S!");
//     while (1); // do nothing
//   }
  
//   Serial.println("I2S PDM initialized");
  
//   // Initialize energy buffer
//   for (int i = 0; i < ENERGY_BUFFER_SIZE; i++) {
//     energyBuffer[i] = 0.0;
//   }
  
//   // Initialize message data
//   memset(messageData, 0, MESSAGE_SIZE);
  
//   // Initialize BLE
//   BLEDevice::init("ESP32_Beat_Server");
  
//   // Create the BLE Server
//   pServer = BLEDevice::createServer();
//   pServer->setCallbacks(new MyServerCallbacks());
  
//   // Create the BLE Service
//   BLEService *pService = pServer->createService(SERVICE_UUID);
  
//   // Create a BLE Characteristic
//   pCharacteristic = pService->createCharacteristic(
//                       CHARACTERISTIC_UUID,
//                       BLECharacteristic::PROPERTY_READ   |
//                       BLECharacteristic::PROPERTY_WRITE  |
//                       BLECharacteristic::PROPERTY_NOTIFY |
//                       BLECharacteristic::PROPERTY_INDICATE
//                     );
  
//   // Create a BLE Descriptor
//   pCharacteristic->addDescriptor(new BLE2902());
  
//   // Start the service
//   pService->start();
  
//   // Start advertising
//   BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//   pAdvertising->addServiceUUID(SERVICE_UUID);
//   pAdvertising->setScanResponse(true);
//   pAdvertising->setMinPreferred(0x06);
//   pAdvertising->setMinPreferred(0x12);
//   BLEDevice::startAdvertising();
  
//   Serial.println("BLE server ready, waiting for connections...");
// }

// void loop() {
//   // Read samples from I2S
//   int sample = I2S.read();
  
//   if (sample && sample != -1 && sample != 1) {
//     // Add sample to buffer
//     sampleBuffer[bufferIndex++] = (int16_t)sample;
    
//     // When buffer is full, process it
//     if (bufferIndex >= BUFFER_SIZE) {
//       // Apply noise reduction
//       applyNoiseReduction(sampleBuffer, processedBuffer, BUFFER_SIZE);
      
//       // Detect beats
//       beatDetected = detectBeat(processedBuffer, BUFFER_SIZE);
      
//       if (beatDetected && deviceConnected) {
//         // Send beat data to client
//         sendBeatData(beatStrength);
//       }
      
//       // If connected, optionally send a downsampled version of the audio
//       if (deviceConnected) {
//         // Send compressed representation of the signal
//         // This reduces the BLE data rate while still providing information about the signal
        
//         // Send just 10 representative samples (every 50th sample)
//         for (int i = 0; i < 10; i++) {
//           int idx = i * 50;
//           if (idx < BUFFER_SIZE) {
//             int16_t value = processedBuffer[idx];
//             pCharacteristic->setValue((uint8_t*)&value, sizeof(int16_t));
//             pCharacteristic->notify();
//             delay(5); // Small delay to avoid overwhelming the BLE stack
//           }
//         }
//       }
      
//       // Reset buffer index
//       bufferIndex = 0;
//     }
//   }
  
//   // Handle BLE connection status changes
//   if (!deviceConnected && oldDeviceConnected) {
//     delay(500); // Give the bluetooth stack time to get ready
//     pServer->startAdvertising(); // Restart advertising
//     Serial.println("Started advertising again");
//     oldDeviceConnected = deviceConnected;
//   }
  
//   if (deviceConnected && !oldDeviceConnected) {
//     oldDeviceConnected = deviceConnected;
//   }
// }

#include <ESP_I2S.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>
#include <math.h>

// I2S settings
I2SClass I2S;
#define SAMPLE_RATE 16000
#define PDM_DATA_PIN 41
#define PDM_CLOCK_PIN 42

// Buffer settings
#define BUFFER_SIZE 512  // Larger buffer for better frequency analysis
int16_t sampleBuffer[BUFFER_SIZE];
int16_t processedBuffer[BUFFER_SIZE]; // Buffer for processed samples
int bufferIndex = 0;

// BLE server and characteristic definitions
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Define UUIDs for the service and characteristic
#define SERVICE_UUID "0cebcba9-4624-4999-9cec-5debf01d8ecb"
#define CHARACTERISTIC_UUID "65f04fbf-01ac-4954-a999-af6b77c65fdf"

// Beat detection parameters
#define ENERGY_SMOOTHING 0.3      // Smoothing factor for energy calculation
#define BEAT_THRESHOLD 1.1        // Energy must be this times the average to trigger a beat
#define MIN_BEAT_INTERVAL 200     // Minimum time between beats (ms)
#define BEAT_HOLD_TIME 100        // Hold the beat indicator for this long (ms)

// Energy buffer for beat detection
#define ENERGY_BUFFER_SIZE 32
float energyBuffer[ENERGY_BUFFER_SIZE];
int energyIndex = 0;
float currentEnergy = 0;
float averageEnergy = 0;
bool beatDetected = false;
unsigned long lastBeatTime = 0;
unsigned long beatIndicatorTime = 0;
float beatStrength = 0.0;

// Noise reduction parameters
#define NOISE_FLOOR 300            // Initial noise floor estimate
#define NOISE_ADAPT_RATE 0.01      // Rate to adapt noise floor estimate
float noiseFloor = NOISE_FLOOR;

// Response variables to BLE requests
#define MESSAGE_SIZE 20
uint8_t messageData[MESSAGE_SIZE]; // Data to send over BLE

// Server callbacks class to detect connection status
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected");
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected");
    }
};

// Remove DC offset and reduce noise
// void applyNoiseReduction(int16_t* samples, int16_t* output, int numSamples) {
//   // Calculate the average (DC offset)
//   int32_t sum = 0;
//   for (int i = 0; i < numSamples; i++) {
//     sum += samples[i];
//   }
//   int16_t dcOffset = sum / numSamples;
  
//   // Apply DC offset removal and noise gate
//   for (int i = 0; i < numSamples; i++) {
//     // Remove DC offset
//     output[i] = samples[i] - dcOffset;
    
//     // Apply noise gate
//     if (abs(output[i]) < noiseFloor) {
//       output[i] = 0;
//     }
//   }
  
//   // Update noise floor estimate (adapt to environment)
//   int16_t currentNoiseLevel = 0;
//   for (int i = 0; i < numSamples; i++) {
//     currentNoiseLevel += abs(output[i]);
//   }
//   currentNoiseLevel /= numSamples;
  
//   // Only adapt noise floor during quiet periods
//   if (currentNoiseLevel < noiseFloor * 2) {
//     noiseFloor = (1 - NOISE_ADAPT_RATE) * noiseFloor + NOISE_ADAPT_RATE * currentNoiseLevel;
//   }
// }
#define LP_FILTER_ALPHA 0.1     // 滤波系数，值越小滤波越强 (0.05-0.2适合beat检测)

// 滤波状态变量
static float lastFilteredSample = 0;

// 应用简化版低通滤波器
void applySimpleLowPassFilter(int16_t* input, int16_t* output, int numSamples) {
  for(int i = 0; i < numSamples; i++) {
    // 简单IIR滤波: y[n] = alpha*x[n] + (1-alpha)*y[n-1]
    float filtered = LP_FILTER_ALPHA * input[i] + (1.0 - LP_FILTER_ALPHA) * lastFilteredSample;
    lastFilteredSample = filtered;
    output[i] = (int16_t)filtered;
  }
}

// 改进的噪声消除函数，使用简化低通滤波器
void applyNoiseReduction(int16_t* samples, int16_t* output, int numSamples) {
  // 临时缓冲区用于低通滤波
  static int16_t filteredBuffer[BUFFER_SIZE];
  
  // 应用简化低通滤波器
  applySimpleLowPassFilter(samples, filteredBuffer, numSamples);
  
  // 计算平均值(DC偏移)
  int32_t sum = 0;
  for (int i = 0; i < numSamples; i++) {
    sum += filteredBuffer[i];
  }
  int16_t dcOffset = sum / numSamples;
  
  // 计算样本标准差以自动调整增益
  float sumSquares = 0;
  for (int i = 0; i < numSamples; i++) {
    int16_t centered = filteredBuffer[i] - dcOffset;
    sumSquares += centered * centered;
  }
  float stdDev = sqrt(sumSquares / numSamples);
  
  // 应用DC偏移移除、增益和噪声门限
  for (int i = 0; i < numSamples; i++) {
    // 移除DC偏移
    float sample = filteredBuffer[i] - dcOffset;
    // 应用软噪声门限(soft noise gate)而不是硬切
    if (abs(sample) < noiseFloor*1.2) {
      // 平滑噪声门限，而不是直接设为0
      float ratio = abs(sample) / noiseFloor;
      sample *= ratio * ratio;  // 平方关系使低于门限的信号平滑衰减
    }
    
    output[i] = (int16_t)sample;
  }
  
  // 更新噪声门限估计(适应环境)
  int16_t currentNoiseLevel = 0;
  for (int i = 0; i < numSamples; i++) {
    currentNoiseLevel += abs(output[i]);
  }
  currentNoiseLevel /= numSamples;
  
  // 仅在安静期间调整噪声门限
  if (currentNoiseLevel < noiseFloor * 2) {
    noiseFloor = (1 - NOISE_ADAPT_RATE) * noiseFloor + NOISE_ADAPT_RATE * currentNoiseLevel;
  }
}

// Detect beats using energy-based algorithm
bool detectBeat(int16_t* samples, int numSamples) {
  // Calculate signal energy (RMS)
  float energy = 0;
  for (int i = 0; i < numSamples; i++) {
    energy += (float)samples[i] * samples[i];
  }
  energy = sqrt(energy / numSamples);
  
  // Smooth the energy value
  currentEnergy = (1 - ENERGY_SMOOTHING) * currentEnergy + ENERGY_SMOOTHING * energy;
  
  // Store in circular buffer
  energyBuffer[energyIndex] = currentEnergy;
  energyIndex = (energyIndex + 1) % ENERGY_BUFFER_SIZE;
  
  // Calculate average energy
  float sum = 0;
  for (int i = 0; i < ENERGY_BUFFER_SIZE; i++) {
    sum += energyBuffer[i];
  }
  averageEnergy = sum / ENERGY_BUFFER_SIZE;
  
  // Check if enough time has passed since the last beat
  unsigned long currentTime = millis();
  if (currentTime - lastBeatTime < MIN_BEAT_INTERVAL) {
    return false;
  }
  
  // Beat detection logic
  if (currentEnergy > averageEnergy * BEAT_THRESHOLD && currentEnergy > noiseFloor * 3) {
    lastBeatTime = currentTime;
    beatIndicatorTime = currentTime;
    
    // Calculate beat strength (normalized)
    beatStrength = constrain((currentEnergy - averageEnergy) / averageEnergy, 0.0, 1.0);
    
    return true;
  }
  
  return false;
}

// Send beat data to client
void sendBeatData(float strength) {
  if (!deviceConnected) return;
  
  // Create a structured message: [flag byte, strength float (4 bytes)]
  messageData[0] = 0x01; // Beat indicator flag
  
  // Convert float to bytes
  memcpy(&messageData[1], &strength, sizeof(float));
  
  // Send the data
  pCharacteristic->setValue(messageData, 5); // 1 flag byte + 4 bytes float
  pCharacteristic->notify();
  
  Serial.print("Beat detected! Strength: ");
  Serial.println(strength);
}

void setup() {
  // Initialize serial
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  Serial.println("ESP32 PDM Beat Detection Server");

  // Setup I2S for PDM microphone
  I2S.setPinsPdmRx(PDM_CLOCK_PIN, PDM_DATA_PIN);
  
  if (!I2S.begin(I2S_MODE_PDM_RX, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // do nothing
  }
  
  Serial.println("I2S PDM initialized");
  
  // Initialize energy buffer
  for (int i = 0; i < ENERGY_BUFFER_SIZE; i++) {
    energyBuffer[i] = 0.0;
  }
  
  // Initialize message data
  memset(messageData, 0, MESSAGE_SIZE);
  
  // Initialize BLE
  BLEDevice::init("ESP32_Beat_Server");
  
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());
  
  // Start the service
  pService->start();
  
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE server ready, waiting for connections...");
}

void loop() {
  // Read samples from I2S
  int sample = I2S.read();
  
  if (sample && sample != -1 && sample != 1) {
    // Add sample to buffer
    sampleBuffer[bufferIndex++] = (int16_t)sample;
    
    // When buffer is full, process it
    if (bufferIndex >= BUFFER_SIZE) {
      // Apply noise reduction
      applyNoiseReduction(sampleBuffer, processedBuffer, BUFFER_SIZE);
      
      // Detect beats
      beatDetected = detectBeat(processedBuffer, BUFFER_SIZE);
      
      if (beatDetected && deviceConnected) {
        // Send beat data to client
        sendBeatData(beatStrength);
      }
      
      // If connected, optionally send a downsampled version of the audio
      if (deviceConnected) {
        // Send compressed representation of the signal
        // This reduces the BLE data rate while still providing information about the signal
        
        // Send just 10 representative samples (every 50th sample)
        for (int i = 0; i < 10; i++) {
          int idx = i * 50;
          if (idx < BUFFER_SIZE) {
            int16_t value = processedBuffer[idx];
            pCharacteristic->setValue((uint8_t*)&value, sizeof(int16_t));
            pCharacteristic->notify();
            delay(5); // Small delay to avoid overwhelming the BLE stack
          }
        }
      }
      
      // Reset buffer index
      bufferIndex = 0;
    }
  }
  
  // Handle BLE connection status changes
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Give the bluetooth stack time to get ready
    pServer->startAdvertising(); // Restart advertising
    Serial.println("Started advertising again");
    oldDeviceConnected = deviceConnected;
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}