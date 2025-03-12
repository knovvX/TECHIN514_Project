// #include "bluetooth_manager.h"
// #include <Arduino.h>

// // BLE server pointer
// static BLEClient* pClient = nullptr;
// static BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;
// static BLEAdvertisedDevice* myDevice = nullptr;
// static bool connected = false;
// static bool doScan = false;
// static float latestWaveformValue = 0.0;
// static char statusMessage[64] = "Disconnected";
// static BluetoothEvent currentEvent = BT_EVENT_NONE;


// // Buffer for received data
// static int16_t receivedBuffer[BT_DATA_BUFFER_SIZE];

// // Callback class for notifications
// class MyClientCallback : public BLEClientCallbacks {
//   void onConnect(BLEClient* pclient) {
//     connected = true;
//     currentEvent = BT_EVENT_CONNECTED;
//     snprintf(statusMessage, sizeof(statusMessage), "Connected to server");
//     Serial.println(statusMessage);
//   }

//   void onDisconnect(BLEClient* pclient) {
//     connected = false;
//     currentEvent = BT_EVENT_DISCONNECTED;
//     snprintf(statusMessage, sizeof(statusMessage), "Disconnected from server");
//     Serial.println(statusMessage);
//     doScan = true;
//   }
// };

// // Notification callback function
// static void notifyCallback(
//   BLERemoteCharacteristic* pBLERemoteCharacteristic,
//   uint8_t* pData,
//   size_t length,
//   bool isNotify) {

//   if (length > 0 && length <= BT_DATA_BUFFER_SIZE * sizeof(int16_t)) {
//     // Copy data to our buffer
//     memcpy(receivedBuffer, pData, length);

//     // Extract the first value for simple display
//     // This assumes the data is in int16_t format as sent by the server
//     // Reconstruct the 16-bit value from high and low bytes
//     int16_t rawValue = (receivedBuffer[0] << 8) | receivedBuffer[1];

//     // Normalize to [-1.0, 1.0]
//     // For signed 16-bit integers (int16_t), the range is -32768 to 32767
//     latestWaveformValue = (float)rawValue / 32767.0f;
//     // latestWaveformValue = (float)receivedBuffer[0] / 32767.0;  // Normalize to [-1.0, 1.0]

//     currentEvent = BT_EVENT_DATA_RECEIVED;

//     // Debug output
//     // Serial.print("Received data, length: ");
//     // Serial.print(length);
//     // Serial.print(", first value: ");
//     // Serial.println(receivedBuffer[0]);
//   }
// }

// // Scan for BLE servers and find the target device
// class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
//   void onResult(BLEAdvertisedDevice advertisedDevice) {
//     Serial.print("BLE Device found: ");
//     Serial.println(advertisedDevice.toString().c_str());

//     // Check if the device has our service UUID
//     if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {

//       BLEDevice::getScan()->stop();
//       myDevice = new BLEAdvertisedDevice(advertisedDevice);
//       doScan = false;

//       Serial.println("Found target device, scan stopped");
//     }
//   }
// };

// // Connect to the BLE server
// bool connectToServer() {
//   Serial.print("Connecting to ");
//   Serial.println(myDevice->getAddress().toString().c_str());

//   // Create client and set callbacks
//   pClient = BLEDevice::createClient();
//   pClient->setClientCallbacks(new MyClientCallback());

//   // Connect to the remote device
//   if (!pClient->connect(myDevice)) {
//     Serial.println("Failed to connect");
//     return false;
//   }
//   Serial.println("Connected to the BLE server");

//   // Get the service
//   BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
//   if (pRemoteService == nullptr) {
//     Serial.println("Failed to find service UUID");
//     pClient->disconnect();
//     return false;
//   }
//   Serial.println("Found the service");

//   // Get the characteristic
//   pRemoteCharacteristic = pRemoteService->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID));
//   if (pRemoteCharacteristic == nullptr) {
//     Serial.println("Failed to find characteristic UUID");
//     pClient->disconnect();
//     return false;
//   }
//   Serial.println("Found the characteristic");

//   // Register for notifications if supported
//   if (pRemoteCharacteristic->canNotify()) {
//     pRemoteCharacteristic->registerForNotify(notifyCallback);
//     Serial.println("Registered for notifications");
//   }

//   connected = true;
//   snprintf(statusMessage, sizeof(statusMessage), "Connected to audio source");
//   return true;
// }

// bool initBluetooth() {
//   // Initialize BLE
//   BLEDevice::init(BT_DEVICE_NAME);

//   // Create scan object
//   BLEScan* pBLEScan = BLEDevice::getScan();
//   pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
//   pBLEScan->setInterval(1349);
//   pBLEScan->setWindow(449);
//   pBLEScan->setActiveScan(true);

//   // Start scanning
//   doScan = true;

//   Serial.println("Bluetooth initialized, scan started");
//   return true;
// }

// void updateBluetooth() {
//   // If disconnected and we need to scan
//   if (doScan) {
//     BLEDevice::getScan()->start(5, false);  // 5 seconds scan
//     Serial.println("Scanning for BLE devices...");
//     doScan = false;
//   }

//   // If we found a device, try to connect
//   if (!connected && myDevice != nullptr) {
//     if (connectToServer()) {
//       Serial.println("Connection successful");
//     } else {
//       Serial.println("Connection failed, will retry");
//       delay(1000);  // Wait a bit before retrying
//       doScan = true;
//     }
//   }

//   // Reset the event after processing
//   if (currentEvent != BT_EVENT_NONE) {
//     BluetoothEvent tempEvent = currentEvent;
//     currentEvent = BT_EVENT_NONE;
//     // Keep the latest event for one cycle
//     currentEvent = tempEvent;
//   }
// }

// BluetoothEvent checkBluetoothEvent() {
//   BluetoothEvent tempEvent = currentEvent;
//   currentEvent = BT_EVENT_NONE;
//   return tempEvent;
// }

// void sendBluetoothData(const char* data) {
//   // Not implemented for receiver side
//   Serial.println("Warning: sendBluetoothData not implemented for receiver");
// }

// float getBluetoothWaveformValue() {
//   return latestWaveformValue;
// }

// bool isBluetoothConnected() {
//   return connected;
// }

// const char* getBluetoothStatusMessage() {
//   return statusMessage;
// }
#include "bluetooth_manager.h"
#include <Arduino.h>

// BLE server pointer
static BLEClient* pClient = nullptr;
static BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;
static BLEAdvertisedDevice* myDevice = nullptr;
static bool connected = false;
static bool doScan = false;
static float latestWaveformValue = 0.0;
static char statusMessage[64] = "Disconnected";
static BluetoothEvent currentEvent = BT_EVENT_NONE;

// 扫描和省电管理变量
static unsigned long lastScanTime = 0;
static const unsigned long RESCAN_INTERVAL = 30000;  // 30秒后重新扫描
static unsigned long lastConnectionAttempt = 0;
static const unsigned long CONNECTION_RETRY_INTERVAL = 5000;  // 5秒后重试连接
static int scanAttemptCount = 0;
static const int MAX_SCAN_ATTEMPTS = 5;                   // 最大扫描尝试次数
static const unsigned long POWER_SAVE_INTERVAL = 300000;  // 5分钟省电模式
static bool inPowerSaveMode = false;
static unsigned long powerSaveModeStartTime = 0;

static const int MAX_BEATS = 30;
static unsigned long beatTimes[MAX_BEATS];
static int beatCount = 0;
static int oldestBeatIndex = 0;
static float lastReportedBPM = 0;
const float SMOOTHING_FACTOR = 0.6;   // 数值越低，平滑效果越强
static float lastBeatStrength = 0.0;  // 保存最近的beat强度


// Buffer for received data
static int16_t receivedBuffer[BT_DATA_BUFFER_SIZE];

// Callback class for notifications
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
    currentEvent = BT_EVENT_CONNECTED;
    snprintf(statusMessage, sizeof(statusMessage), "Connected to server");
    Serial.println(statusMessage);
    // 连接成功时重置所有扫描计数和省电模式
    scanAttemptCount = 0;
    inPowerSaveMode = false;
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    currentEvent = BT_EVENT_DISCONNECTED;
    snprintf(statusMessage, sizeof(statusMessage), "Disconnected from server");
    Serial.println(statusMessage);
    doScan = true;
    lastScanTime = millis();  // 记录断开连接时的时间
    // 断开连接时重置扫描计数
    scanAttemptCount = 0;
    inPowerSaveMode = false;
  }
};

// Notification callback function
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {

  if (length > 0 && length <= BT_DATA_BUFFER_SIZE * sizeof(int16_t)) {
    // Copy data to our buffer
    memcpy(receivedBuffer, pData, length);

    // Extract the first value for simple display
    // This assumes the data is in int16_t format as sent by the server
    // Reconstruct the 16-bit value from high and low bytes
    int16_t rawValue = (receivedBuffer[0] << 8) | receivedBuffer[1];

    // Normalize to [-1.0, 1.0]
    // For signed 16-bit integers (int16_t), the range is -32768 to 32767
    latestWaveformValue = (float)rawValue / 32767.0f;
    handleBluetoothData(pData, length);

    currentEvent = BT_EVENT_DATA_RECEIVED;
  }
}

// Scan for BLE servers and find the target device
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // Check if the device has our service UUID
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doScan = false;
      // 找到目标设备，重置扫描计数
      scanAttemptCount = 0;
      inPowerSaveMode = false;
      Serial.println("Found target device, scan stopped");
    }
  }
};

// Connect to the BLE server
bool connectToServer() {
  Serial.print("Connecting to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  // Create client and set callbacks
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remote device
  if (!pClient->connect(myDevice)) {
    Serial.println("Failed to connect");
    return false;
  }
  Serial.println("Connected to the BLE server");

  // Get the service
  BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
  if (pRemoteService == nullptr) {
    Serial.println("Failed to find service UUID");
    pClient->disconnect();
    return false;
  }
  Serial.println("Found the service");

  // Get the characteristic
  pRemoteCharacteristic = pRemoteService->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID));
  if (pRemoteCharacteristic == nullptr) {
    Serial.println("Failed to find characteristic UUID");
    pClient->disconnect();
    return false;
  }
  Serial.println("Found the characteristic");

  // Register for notifications if supported
  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(notifyCallback);
    Serial.println("Registered for notifications");
  }

  connected = true;
  snprintf(statusMessage, sizeof(statusMessage), "Connected to audio source");
  return true;
}

bool initBluetooth() {
  // Initialize BLE
  BLEDevice::init(BT_DEVICE_NAME);

  // Create scan object
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);

  // Start scanning
  doScan = true;
  lastScanTime = millis();  // 记录初始扫描时间
  scanAttemptCount = 0;
  inPowerSaveMode = false;

  Serial.println("Bluetooth initialized, scan started");
  return true;
}

void updateBluetooth() {
  unsigned long currentTime = millis();
  if (connected) {
    return;
  }
  // 省电模式处理
  if (inPowerSaveMode) {
    // 检查是否应该退出省电模式
    if (currentTime - powerSaveModeStartTime >= POWER_SAVE_INTERVAL) {
      inPowerSaveMode = false;
      scanAttemptCount = 0;  // 重置扫描计数
      doScan = true;         // 退出省电模式后进行一次扫描
      Serial.println("Exiting power save mode, resuming scans");
      snprintf(statusMessage, sizeof(statusMessage), "Resuming BT scans");
    } else {
      // 在省电模式中，跳过任何扫描或连接尝试
      return;
    }
  }

  // 如果未连接且未在扫描
  if (!connected && !doScan) {
    // 如果没有设备或者长时间未尝试连接，考虑进行新的扫描
    if (myDevice == nullptr && (currentTime - lastScanTime >= RESCAN_INTERVAL)) {
      // 检查是否已达到最大扫描次数
      if (scanAttemptCount >= MAX_SCAN_ATTEMPTS) {
        // 进入省电模式
        if (!inPowerSaveMode) {
          inPowerSaveMode = true;
          powerSaveModeStartTime = currentTime;
          Serial.println("Maximum scan attempts reached, entering power save mode");
          snprintf(statusMessage, sizeof(statusMessage), "Power save mode");
        }
      } else {
        // 未达到最大扫描次数，继续扫描
        Serial.print("Scan attempt #");
        Serial.print(scanAttemptCount + 1);
        Serial.println(" of maximum attempts");
        doScan = true;
        lastScanTime = currentTime;
      }
    }
    // 如果有设备但连接失败，过一段时间后重试
    else if (myDevice != nullptr && (currentTime - lastConnectionAttempt >= CONNECTION_RETRY_INTERVAL)) {
      Serial.println("Connection retry after interval...");
      lastConnectionAttempt = currentTime;
    }
  }

  // 如果需要扫描且不在省电模式
  if (doScan && !inPowerSaveMode) {
    BLEDevice::getScan()->start(5, false);  // 5秒扫描
    Serial.println("Scanning for BLE devices...");
    doScan = false;
    lastScanTime = currentTime;
    scanAttemptCount++;  // 增加扫描计数
    snprintf(statusMessage, sizeof(statusMessage), "Scanning #%d/%d",
             scanAttemptCount, MAX_SCAN_ATTEMPTS);
  }

  // 如果找到设备，尝试连接
  if (!connected && myDevice != nullptr) {
    if (connectToServer()) {
      Serial.println("Connection successful");
      lastConnectionAttempt = currentTime;
      // 成功连接后重置扫描计数
      scanAttemptCount = 0;
    } else {
      Serial.println("Connection failed, will retry");
      lastConnectionAttempt = currentTime;
      delay(1000);  // 连接失败后稍等片刻

      // 连接失败多次后，考虑重新扫描
      static int failedAttempts = 0;
      failedAttempts++;

      if (failedAttempts >= 3) {
        doScan = true;
        failedAttempts = 0;

        // 释放旧设备资源，以便扫描新设备
        if (myDevice != nullptr) {
          delete myDevice;
          myDevice = nullptr;
        }
      }
    }
  }

  // 重置事件
  if (currentEvent != BT_EVENT_NONE) {
    BluetoothEvent tempEvent = currentEvent;
    currentEvent = BT_EVENT_NONE;
    currentEvent = tempEvent;
  }
}

BluetoothEvent checkBluetoothEvent() {
  BluetoothEvent tempEvent = currentEvent;
  currentEvent = BT_EVENT_NONE;
  return tempEvent;
}

void sendBluetoothData(const char* data) {
  // Not implemented for receiver side
  Serial.println("Warning: sendBluetoothData not implemented for receiver");
}

float getBluetoothWaveformValue() {
  return latestWaveformValue;
}

bool isBluetoothConnected() {
  return connected;
}

const char* getBluetoothStatusMessage() {
  return statusMessage;
}

// 新增函数：手动退出省电模式
void exitBluetoothPowerSaveMode() {
  if (inPowerSaveMode) {
    inPowerSaveMode = false;
    scanAttemptCount = 0;
    doScan = true;
    Serial.println("Manually exiting power save mode");
    snprintf(statusMessage, sizeof(statusMessage), "Scanning resumed");
  }
}
void handleBluetoothData(uint8_t* data, size_t length) {
  // 检查是否是beat标志(0x01)
  if (length >= 5 && data[0] == 0x01) {
    // 提取beat强度
    float strength;
    memcpy(&strength, &data[1], sizeof(float));
    lastBeatStrength = strength;

    // 记录beat
    recordBeatFromBluetooth();

    // 调试输出
    Serial.print("Beat received! Strength: ");
    Serial.println(strength);
  }
}

// 在收到蓝牙beat信号时调用此函数
void recordBeatFromBluetooth() {
  // 记录当前时间作为beat时间
  unsigned long currentTime = millis();

  // 存储beat时间并更新索引
  beatTimes[oldestBeatIndex] = currentTime;
  oldestBeatIndex = (oldestBeatIndex + 1) % MAX_BEATS;
  if (beatCount < MAX_BEATS) beatCount++;
}

// 获取最近的beat强度
float getBluetoothBeatStrength() {
  return lastBeatStrength;
}

// BPM计算函数，在需要BPM值时调用
float calculateBPMFromBeats() {
  // 测量窗口大小（毫秒）
  const unsigned long measurementWindow = 10000;  // 10秒滑动窗口

  // 平滑处理参数
  const float MIN_BPM = 40.0;
  const float MAX_BPM = 220.0;

  // 当前时间
  unsigned long currentTime = millis();
  // Serial.print("beat count: ");
  // Serial.println(beatCount);
  // 计算BPM，需要至少2个beat
  if (beatCount < 2) return MIN_BPM;
  // Serial.print("beat count: ");
  // Serial.println(beatCount);

  // 找出在测量窗口内的最早beat
  int validBeatCount = 0;
  int oldestValidBeat = oldestBeatIndex;

  // for (int i = 0; i < beatCount; i++) {
  //     int index = (oldestBeatIndex - i + MAX_BEATS) % MAX_BEATS;
  //     if (currentTime - beatTimes[index] <= measurementWindow) {
  //         validBeatCount++;
  //         oldestValidBeat = index;
  //     } else {
  //         break; // 找到一个窗口外的beat
  //     }
  // }
  // 最新节拍的索引是 (oldestBeatIndex - 1)，但需要处理负数情况
  int newestBeatIndex = (oldestBeatIndex > 0) ? (oldestBeatIndex - 1) : (MAX_BEATS - 1);

  for (int i = 0; i < beatCount; i++) {
    int index = (newestBeatIndex - i + MAX_BEATS) % MAX_BEATS;
    if (currentTime - beatTimes[index] <= measurementWindow) {
      validBeatCount++;
      oldestValidBeat = index;
    } else {
      break;  // 找到一个窗口外的节拍
    }
  }
  Serial.print("valid count: ");
  Serial.println(validBeatCount);
  if (validBeatCount < 2) return MIN_BPM;  // 需要至少2个有效beat

  // 计算连续beat之间的间隔
  int validIntervals = validBeatCount - 1;
  unsigned long intervals[validIntervals];
  int intervalIndex = 0;

  for (int i = 0; i < validBeatCount - 1; i++) {
    int currentIndex = (oldestValidBeat + i) % MAX_BEATS;
    int nextIndex = (oldestValidBeat + i + 1) % MAX_BEATS;
    unsigned long interval = beatTimes[nextIndex] - beatTimes[currentIndex];

    // 只包含合理的间隔（避免计数错误）
    if (interval > 200 && interval < 2000) {  // 30-300 BPM
      intervals[intervalIndex++] = interval;
    }
  }

  // 需要至少一个有效间隔
  if (intervalIndex == 0) {
    if (lastReportedBPM > 0) return lastReportedBPM;
    return MIN_BPM;
  }

  // 使用中值滤波处理间隔
  if (intervalIndex > 2) {
    // 排序间隔（对于小数组，冒泡排序足够了）
    for (int i = 0; i < intervalIndex - 1; i++) {
      for (int j = 0; j < intervalIndex - i - 1; j++) {
        if (intervals[j] > intervals[j + 1]) {
          unsigned long temp = intervals[j];
          intervals[j] = intervals[j + 1];
          intervals[j + 1] = temp;
        }
      }
    }

    // 使用中值间隔计算BPM
    unsigned long medianInterval;
    if (intervalIndex % 2 == 0) {
      medianInterval = (intervals[intervalIndex / 2] + intervals[intervalIndex / 2 - 1]) / 2;
    } else {
      medianInterval = intervals[intervalIndex / 2];
    }

    // 转换为BPM
    float rawBPM = 60000.0 / medianInterval;

    // 限制为有效范围
    if (rawBPM < MIN_BPM) rawBPM = MIN_BPM;
    if (rawBPM > MAX_BPM) rawBPM = MAX_BPM;

    // 应用指数平滑
    float smoothedBPM;
    if (lastReportedBPM > 0) {
      smoothedBPM = (SMOOTHING_FACTOR * rawBPM) + ((1 - SMOOTHING_FACTOR) * lastReportedBPM);
    } else {
      smoothedBPM = rawBPM;  // 第一次读取
    }

    lastReportedBPM = smoothedBPM;
    return smoothedBPM;
  } else {
    // 如果只有1-2个间隔，使用平均方法
    unsigned long timeSpan = currentTime - beatTimes[oldestValidBeat];
    float rawBPM = validIntervals * 60000.0 / timeSpan;

    // 限制为有效范围
    if (rawBPM < MIN_BPM) rawBPM = MIN_BPM;
    if (rawBPM > MAX_BPM) rawBPM = MAX_BPM;

    // 应用指数平滑
    float smoothedBPM;
    if (lastReportedBPM > 0) {
      smoothedBPM = (SMOOTHING_FACTOR * rawBPM) + ((1 - SMOOTHING_FACTOR) * lastReportedBPM);
    } else {
      smoothedBPM = rawBPM;  // 第一次读取
    }

    lastReportedBPM = smoothedBPM;
    return smoothedBPM;
  }
}
