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


// Buffer for received data
static int16_t receivedBuffer[BT_DATA_BUFFER_SIZE];

// Callback class for notifications
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
    currentEvent = BT_EVENT_CONNECTED;
    snprintf(statusMessage, sizeof(statusMessage), "Connected to server");
    Serial.println(statusMessage);
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    currentEvent = BT_EVENT_DISCONNECTED;
    snprintf(statusMessage, sizeof(statusMessage), "Disconnected from server");
    Serial.println(statusMessage);
    doScan = true;
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
    // latestWaveformValue = (float)receivedBuffer[0] / 32767.0;  // Normalize to [-1.0, 1.0]

    currentEvent = BT_EVENT_DATA_RECEIVED;

    // Debug output
    Serial.print("Received data, length: ");
    Serial.print(length);
    Serial.print(", first value: ");
    Serial.println(receivedBuffer[0]);
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

  Serial.println("Bluetooth initialized, scan started");
  return true;
}

void updateBluetooth() {
  // If disconnected and we need to scan
  if (doScan) {
    BLEDevice::getScan()->start(5, false);  // 5 seconds scan
    Serial.println("Scanning for BLE devices...");
    doScan = false;
  }

  // If we found a device, try to connect
  if (!connected && myDevice != nullptr) {
    if (connectToServer()) {
      Serial.println("Connection successful");
    } else {
      Serial.println("Connection failed, will retry");
      delay(1000);  // Wait a bit before retrying
      doScan = true;
    }
  }

  // Reset the event after processing
  if (currentEvent != BT_EVENT_NONE) {
    BluetoothEvent tempEvent = currentEvent;
    currentEvent = BT_EVENT_NONE;
    // Keep the latest event for one cycle
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