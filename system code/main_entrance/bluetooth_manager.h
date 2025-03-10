#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// Define constants for Bluetooth communication
#define BT_DEVICE_NAME "ESP32WaveformDisplay"
#define BT_DATA_BUFFER_SIZE 64

// BLE UUIDs
#define SERVICE_UUID "0cebcba9-4624-4999-9cec-5debf01d8ecb"
#define CHARACTERISTIC_UUID "65f04fbf-01ac-4954-a999-af6b77c65fdf"

// Bluetooth events
typedef enum {
  BT_EVENT_NONE = 0,        // No event
  BT_EVENT_CONNECTED,       // Bluetooth connected
  BT_EVENT_DISCONNECTED,    // Bluetooth disconnected
  BT_EVENT_DATA_RECEIVED    // New data received
} BluetoothEvent;

// Initialize Bluetooth module
bool initBluetooth();

// Process Bluetooth connection and data
void updateBluetooth();

// Check if there are any Bluetooth events
BluetoothEvent checkBluetoothEvent();

// Send data via Bluetooth
void sendBluetoothData(const char* data);

// Get the latest received waveform data
float getBluetoothWaveformValue();

// Check if Bluetooth is connected
bool isBluetoothConnected();

// Get the most recent connection status message
const char* getBluetoothStatusMessage();

#endif // BLUETOOTH_MANAGER_H