#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <Arduino.h>

// Define motor control pins
#define COIL_A1 D0
#define COIL_A2 D1
#define COIL_B1 D2
#define COIL_B2 D3

// Initialize metronome stepper motor pins
void initMetronome();

// Center the metronome to its middle position
void centerMetronome();

// Update the metronome to swing based on current BPM
// Should be called regularly in the main loop
void updateMetronome(float bpm);

// Set metronome to specific position
// position: 0 (center), -90 to +90 (degrees from center)
void setMetronomePosition(int position);

// Get the current position of the metronome in degrees
int getMetronomePosition();

// Check if metronome should start a new beat
// Returns true if a new beat has started (for sound synchronization)
bool newBeatStarted();

#endif // STEPPER_MOTOR_H