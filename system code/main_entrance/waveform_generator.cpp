#include "waveform_generator.h"

// Waveform data
static int waveform[TOTAL_WIDTH];             // Array to store waveform data
static int waveformPos = 0;                   // Current waveform position
static int baseHeight = SCREEN_HEIGHT / 2;    // Baseline height
static int amplitude = 20;                    // Amplitude

// Waveform generation parameters
static float phase = 0.0;                     // Phase
static float phaseIncrement = 0.1;            // Phase increment - INCREASED for faster movement
static float frequency = 1.0;                 // Waveform frequency
static DataSource dataSource = SOURCE_INTERNAL; // Default data source

// Number of positions to advance per update - ADDED for faster movement
static int positionStepSize = 3;              // Move multiple positions per update

void initWaveform() {
  // Initialize waveform data to baseline height
  for(int i = 0; i < TOTAL_WIDTH; i++) {
    waveform[i] = baseHeight;
  }
  
  // Initialize waveform generation parameters
  phase = 0.0;
  waveformPos = 0;
  
  Serial.println("Waveform generator initialized");
}

void generateRandomWaveform() {
  // Generate multiple data points per call for faster movement
  for(int step = 0; step < positionStepSize; step++) {
    // Generate waveform using sine wave + random noise
    float sinVal = sin(phase) * amplitude;
    int randomValue = random(-5, 6);
    
    // Update phase - faster phase change
    phase += phaseIncrement * frequency;
    
    // Add new data point to waveform array
    waveform[waveformPos] = baseHeight + (int)sinVal + randomValue;
    
    // Limit range
    if (waveform[waveformPos] < 0) waveform[waveformPos] = 0;
    if (waveform[waveformPos] > SCREEN_HEIGHT - 1) waveform[waveformPos] = SCREEN_HEIGHT - 1;
    
    // Update waveform position
    waveformPos = (waveformPos + 1) % TOTAL_WIDTH;
  }
}

void generateBluetoothWaveform(float btValue) {
  // Generate multiple data points for faster movement
  for(int step = 0; step < positionStepSize; step++) {
    // Scale the BLE value to fit the screen
    float scaledValue = btValue * amplitude;
    
    // Add new data point to waveform array
    waveform[waveformPos] = baseHeight + (int)scaledValue;
    
    // Limit range
    if (waveform[waveformPos] < 0) waveform[waveformPos] = 0;
    if (waveform[waveformPos] > SCREEN_HEIGHT - 1) waveform[waveformPos] = SCREEN_HEIGHT - 1;
    
    // Update waveform position
    waveformPos = (waveformPos + 1) % TOTAL_WIDTH;
  }
}

int* getWaveformData() {
  return waveform;
}

int getWaveformPos() {
  return waveformPos;
}

int getBaseHeight() {
  return baseHeight;
}

int getAmplitude() {
  return amplitude;
}

float getFrequency() {
  return frequency;
}

void setAmplitude(int newAmplitude) {
  // Set new amplitude, with bounds checking
  if (newAmplitude > 0 && newAmplitude < SCREEN_HEIGHT / 2) {
    amplitude = newAmplitude;
  }
}

void setFrequency(float newFrequency) {
  // Set new frequency, with bounds checking
  if (newFrequency > 0.1 && newFrequency < 5.0) {
    frequency = newFrequency;
  }
}

// New function to set the position step size for speed control
void setWaveformSpeed(int speed) {
  // Set the number of positions to advance per update
  // Higher values = faster movement
  if (speed >= 1 && speed <= 10) {
    positionStepSize = speed;
  }
}

// Get current waveform speed setting
int getWaveformSpeed() {
  return positionStepSize;
}

void setDataSource(DataSource source) {
  dataSource = source;
}

DataSource getDataSource() {
  return dataSource;
}