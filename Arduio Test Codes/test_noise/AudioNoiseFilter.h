// ESP32麦克风噪音处理库
#include <Arduino.h>

class AudioNoiseFilter {
private:
  // 移动平均滤波器参数
  static const int MA_WINDOW_SIZE = 8;
  int16_t ma_buffer[MA_WINDOW_SIZE];
  int ma_index = 0;
  int32_t ma_sum = 0;
  
  // 中值滤波器参数
  static const int MED_WINDOW_SIZE = 5;
  int16_t med_buffer[MED_WINDOW_SIZE];
  int med_index = 0;
  
  // DC偏移消除
  int32_t dc_sum = 0;
  int dc_count = 0;
  int16_t dc_offset = 0;
  static const int DC_CALIBRATION_SAMPLES = 500;
  bool dc_calibrated = false;
  
  // 阈值滤波参数
  int16_t noise_threshold = 500; // 可以根据环境调整
  
  // 指数平滑滤波器参数
  float alpha = 0.2; // 平滑系数，0-1之间，越小平滑效果越强
  int16_t prev_filtered = 0;

public:
  AudioNoiseFilter() {
    // 初始化所有缓冲区
    for (int i = 0; i < MA_WINDOW_SIZE; i++) {
      ma_buffer[i] = 0;
    }
    
    for (int i = 0; i < MED_WINDOW_SIZE; i++) {
      med_buffer[i] = 0;
    }
  }
  
  // 设置噪声阈值
  void setNoiseThreshold(int16_t threshold) {
    noise_threshold = threshold;
  }
  
  // 设置指数平滑系数
  void setAlpha(float new_alpha) {
    if (new_alpha > 0 && new_alpha < 1) {
      alpha = new_alpha;
    }
  }
  
  // DC偏移校准
  bool calibrateDCOffset(int16_t sample) {
    if (!dc_calibrated) {
      dc_sum += sample;
      dc_count++;
      
      if (dc_count >= DC_CALIBRATION_SAMPLES) {
        dc_offset = dc_sum / dc_count;
        dc_calibrated = true;
        Serial.print("DC Offset calibrated: ");
        Serial.println(dc_offset);
      }
    }
    return dc_calibrated;
  }
  
  // 移除DC偏移
  int16_t removeDCOffset(int16_t sample) {
    return sample - dc_offset;
  }
  
  // 移动平均滤波器
  int16_t applyMovingAverage(int16_t sample) {
    // 从总和中减去即将被替换的样本
    ma_sum -= ma_buffer[ma_index];
    // 添加新样本到缓冲区
    ma_buffer[ma_index] = sample;
    ma_sum += sample;
    // 更新索引
    ma_index = (ma_index + 1) % MA_WINDOW_SIZE;
    // 返回平均值
    return ma_sum / MA_WINDOW_SIZE;
  }
  
  // 中值滤波器
  int16_t applyMedianFilter(int16_t sample) {
    // 存储新样本
    med_buffer[med_index] = sample;
    med_index = (med_index + 1) % MED_WINDOW_SIZE;
    
    // 复制缓冲区进行排序
    int16_t sorted[MED_WINDOW_SIZE];
    for (int i = 0; i < MED_WINDOW_SIZE; i++) {
      sorted[i] = med_buffer[i];
    }
    
    // 简单的冒泡排序
    for (int i = 0; i < MED_WINDOW_SIZE - 1; i++) {
      for (int j = 0; j < MED_WINDOW_SIZE - i - 1; j++) {
        if (sorted[j] > sorted[j + 1]) {
          // 交换
          int16_t temp = sorted[j];
          sorted[j] = sorted[j + 1];
          sorted[j + 1] = temp;
        }
      }
    }
    
    // 返回中值
    return sorted[MED_WINDOW_SIZE / 2];
  }
  
  // 阈值滤波（噪声门限）
  int16_t applyThreshold(int16_t sample) {
    if (abs(sample) < noise_threshold) {
      return 0;
    }
    return sample;
  }
  
  // 指数平滑滤波
  int16_t applyExponentialSmoothing(int16_t sample) {
    int16_t filtered = alpha * sample + (1 - alpha) * prev_filtered;
    prev_filtered = filtered;
    return filtered;
  }
  
  // 组合滤波器 - 返回经过所有滤波处理的样本
  int16_t processAudio(int16_t raw_sample, uint8_t filter_mode = 0xFF) {
    int16_t processed = raw_sample;
    
    // 如果尚未校准DC偏移，进行校准
    if (!dc_calibrated) {
      if (!calibrateDCOffset(raw_sample)) {
        return 0; // 校准期间返回0
      }
    }
    
    // 移除DC偏移
    if (filter_mode & 0x01) {
      processed = removeDCOffset(processed);
    }
    
    // 应用中值滤波器减少脉冲噪声
    if (filter_mode & 0x02) {
      processed = applyMedianFilter(processed);
    }
    
    // 应用移动平均滤波器平滑信号
    if (filter_mode & 0x04) {
      processed = applyMovingAverage(processed);
    }
    
    // 应用指数平滑
    if (filter_mode & 0x08) {
      processed = applyExponentialSmoothing(processed);
    }
    
    // 应用阈值滤波去除底噪
    if (filter_mode & 0x10) {
      processed = applyThreshold(processed);
    }
    
    return processed;
  }
};