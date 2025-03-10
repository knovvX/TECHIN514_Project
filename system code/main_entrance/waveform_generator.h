#ifndef WAVEFORM_GENERATOR_H
#define WAVEFORM_GENERATOR_H

#include <Arduino.h>
#include "display_manager.h"

// Constants
#define TOTAL_WIDTH (SCREEN_WIDTH * 2)  // Total width of waveform data buffer

// Initialize waveform generator
void initWaveform();

// Generate waveform data from internal source (sine + random)
void generateRandomWaveform();

// Generate waveform from Bluetooth data
void generateBluetoothWaveform(float btValue);

// Get waveform data array
int* getWaveformData();

// Get current waveform position
int getWaveformPos();

// Get baseline height
int getBaseHeight();

// Get amplitude value
int getAmplitude();

// Get frequency value
float getFrequency();

// Set amplitude value
void setAmplitude(int newAmplitude);

// Set frequency value
void setFrequency(float newFrequency);

// Set waveform movement speed (1-10)
void setWaveformSpeed(int speed);

// Get current waveform speed
int getWaveformSpeed();

// Set data source
typedef enum {
  SOURCE_INTERNAL = 0,    // Internal random waveform generation
  SOURCE_BLUETOOTH = 1    // Data from Bluetooth
} DataSource;

// Set the data source for the waveform
void setDataSource(DataSource source);

// Get the current data source
DataSource getDataSource();

#endif // WAVEFORM_GENERATOR_H