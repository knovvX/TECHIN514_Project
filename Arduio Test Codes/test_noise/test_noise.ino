#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <driver/i2s.h>
// #include "AudioNoiseFilter.h"  // 假设将噪声过滤代码保存为单独的头文件

// BLE服务和特征UUID
#define SERVICE_UUID "0cebcba9-4624-4999-9cec-5debf01d8ecb"
#define CHARACTERISTIC_UUID "65f04fbf-01ac-4954-a999-af6b77c65fdf"

// I2S配置
#define I2S_MIC_CHANNEL I2S_NUM_0
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_41  // SCK
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_42  // WS
#define I2S_MIC_SERIAL_DATA GPIO_NUM_42  // SD
#define BUFFER_SIZE 128

// 连接状态变量
bool deviceConnected = false;
bool oldDeviceConnected = false;
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

// 创建噪声过滤器实例
AudioNoiseFilter noiseFilter;

// BLE服务器回调
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
  }
};

// 初始化I2S
void i2s_init() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_PIN_NO_CHANGE,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA
  };

  i2s_driver_install(I2S_MIC_CHANNEL, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_MIC_CHANNEL, &pin_config);
  i2s_set_clk(I2S_MIC_CHANNEL, 16000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Audio Streaming with Noise Reduction");

  // 初始化I2S
  i2s_init();

  // 创建BLE设备
  BLEDevice::init("ESP32-MIC-STREAM");

  // 创建BLE服务器
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 创建BLE服务
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // 创建BLE特征
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  // 创建BLE描述符
  pCharacteristic->addDescriptor(new BLE2902());

  // 启动服务
  pService->start();

  // 启动广播
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE device advertising, waiting for connections...");

 
}

void loop() {
  int16_t sample_buffer[BUFFER_SIZE];
  size_t bytes_read = 0;

  // Read I2S microphone data
  i2s_read(I2S_MIC_CHANNEL, sample_buffer, sizeof(sample_buffer), &bytes_read, portMAX_DELAY);

  // Debug I2S reading
  Serial.print("I2S read: ");
  Serial.print(bytes_read);
  Serial.println(" bytes");

  if (bytes_read > 0) {
    // Print first sample value for debugging (connected or not)
    Serial.print("Microphone sample value: ");
    Serial.println(sample_buffer[0]);
    
    // If connected, send data
    if (deviceConnected) {
      Serial.println("Device is connected, sending notification...");
      
      // Send audio data
      pCharacteristic->setValue((uint8_t*)sample_buffer, bytes_read);
      pCharacteristic->notify();
      
      // Debug info
      Serial.print("Notification sent, first sample: ");
      Serial.println(sample_buffer[0]);
    } else {
      Serial.println("No device connected. Cannot send data.");
    }
  }

  // Connection management
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Restarting advertising");
    oldDeviceConnected = deviceConnected;
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    Serial.println("Connection detected");
    oldDeviceConnected = deviceConnected;
  }

  // Short delay
  delay(20);
}