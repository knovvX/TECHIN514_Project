// #include <Arduino.h>
// #include <driver/i2s.h>

// #define I2S_WS_PIN  D4  // LRCLK
// #define I2S_SCK_PIN D5  // BCLK
// #define I2S_SD_PIN  D6  // DOUT

// // I2S 配置
// const i2s_config_t i2s_config = {
//     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),  // 主模式 + 接收
//     .sample_rate = 16000,     // 采样率
//     .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // 使用 32-bit 读取
//     .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // 只用左声道
//     .communication_format = I2S_COMM_FORMAT_STAND_I2S, // 标准 I2S 格式
//     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,   // 中断优先级
//     .dma_buf_count = 8,      // DMA 缓冲区数量
//     .dma_buf_len = 64        // 每个 DMA 缓冲区大小
// };

// const i2s_pin_config_t pin_config = {
//     .bck_io_num = I2S_SCK_PIN,    // BCLK
//     .ws_io_num = I2S_WS_PIN,      // LRCLK
//     .data_out_num = I2S_PIN_NO_CHANGE,  // ESP32 作为 I2S 主设备，不需要输出
//     .data_in_num = I2S_SD_PIN     // DOUT
// };


// void setup() {
//   Serial.begin(115200);

//   // 配置 I2S
//   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
//   i2s_set_pin(I2S_NUM_0, &pin_config);
//   i2s_start(I2S_NUM_0);

//   Serial.println("I2S Initialized...");
// }

// void loop() {
//   int32_t sample_buffer[128];  // 采样缓冲区
//   size_t bytes_read;

//   // 读取 I2S 数据
//   i2s_read(I2S_NUM_0, sample_buffer, sizeof(sample_buffer), &bytes_read, portMAX_DELAY);

//   if (bytes_read > 0) {
//     int num_samples = bytes_read / sizeof(int32_t);
//     for (int i = 0; i < num_samples; i++) {
//       int16_t processed_sample = sample_buffer[i] >> 8;  // 24-bit -> 16-bit
//       Serial.println(processed_sample);  // 输出到 Serial Plotter
//     }
//   }
// }

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
// #include <I2S.h>  // 引入 I2S 库用于麦克风读取
#include <driver/i2s.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 1000;

// 定义麦克风数据的缓冲区
#define I2S_WS_PIN D4   // LRCLK (WS)
#define I2S_SCK_PIN D5  // BCLK
#define I2S_SD_PIN D6   // DOUT

// BLE 服务和特征 UUID
#define SERVICE_UUID "0cebcba9-4624-4999-9cec-5debf01d8ecb"
#define CHARACTERISTIC_UUID "65f04fbf-01ac-4954-a999-af6b77c65fdf"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

// 设置 I2S 麦克风
void setupI2S() {
  // I2S 配置
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),  // 主模式 + 接收
    .sample_rate = 16000,                                 // 采样率
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,         // 使用 32-bit 读取
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,          // 只用左声道
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,    // 标准 I2S 格式
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,             // 中断优先级
    .dma_buf_count = 8,                                   // DMA 缓冲区数量
    .dma_buf_len = 64                                     // 每个 DMA 缓冲区大小
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK_PIN,          // BCLK
    .ws_io_num = I2S_WS_PIN,            // LRCLK
    .data_out_num = I2S_PIN_NO_CHANGE,  // ESP32 作为 I2S 主设备，不需要输出
    .data_in_num = I2S_SD_PIN           // DOUT
  };
  // 配置 I2S
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_start(I2S_NUM_0);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  setupI2S();  // 初始化 I2S 麦克风读取

  BLEDevice::init("XIAO_ESP32S3");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  int32_t sample = 0;
  size_t bytes_read;
  int32_t sample_buffer[128];

  // 读取 I2S 麦克风数据
  // i2s_read(I2S_NUM_0, &sample, sizeof(sample), &bytes_read, portMAX_DELAY);
  i2s_read(I2S_NUM_0, sample_buffer, sizeof(sample_buffer), &bytes_read, portMAX_DELAY);

  if (bytes_read > 0) {
    int num_samples = bytes_read / sizeof(int32_t);

    for (int i = 0; i < num_samples; i++) {
      int16_t processed_sample = sample_buffer[i] >> 8;  // 24-bit -> 16-bit
      Serial.println(processed_sample);                  // 输出到 Serial Plotter

      // 如果有数据，发送 BLE 通知
      if (deviceConnected) {
        String data = String(processed_sample);
        pCharacteristic->setValue(data.c_str());  // 设置 BLE 特征值
        pCharacteristic->notify();                // 发送通知
        Serial.print("Notify value: ");
        Serial.println(data);  // 在串口显示数据
      }
    }
  }

  // 处理设备连接状态
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // 等待蓝牙堆栈准备好
    pServer->startAdvertising();  // 重新开始广播
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    // 设备刚连接时的操作
    oldDeviceConnected = deviceConnected;
  }

  delay(100);
}
