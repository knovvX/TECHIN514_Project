#include "stepper_motor.h"

// Stepper motor step sequence (full step mode)
const int stepSequence[4][4] = {
  { 1, 0, 1, 0 },  // Step 1
  { 0, 1, 1, 0 },  // Step 2
  { 0, 1, 0, 1 },  // Step 3
  { 1, 0, 0, 1 }   // Step 4
};

// Metronome configuration
const int STEPS_PER_REVOLUTION = 200;                        // Standard for most steppers (1.8° per step)
const int STEPS_FOR_180_DEGREES = STEPS_PER_REVOLUTION / 2;  // 100 steps
const int MAX_SWING_STEPS = STEPS_FOR_180_DEGREES / 2;       // 50 steps (90 degrees) to each side

// BPM thresholds for adjusting swing amplitude
const float BPM_THRESHOLD_1 = 120.0;  // Above this, reduce swing
const float BPM_THRESHOLD_2 = 160.0;  // Above this, reduce swing more
const float BPM_THRESHOLD_3 = 200.0;  // Above this, reduce swing even more

// Metronome state
static int currentStepPosition = 0;      // Current physical step position
static int targetStepPosition = 0;       // Target step position
static int currentDegreePosition = 0;    // Current position in degrees
static bool swingDirectionRight = true;  // Current swing direction
static unsigned long lastStepTime = 0;   // Last motor step time
static unsigned long lastBeatTime = 0;   // Last time we processed a beat
static bool beatStarted = false;         // Flag for beat starting
static int swingAmplitude = 90;          // Current swing amplitude (degrees to each side)
static bool motorEnabled = true;         // Flag to enable/disable motor
static int currentStepIndex = 0;         // Current step sequence index

// For timing calculations
static float currentBpm = DEFAULT_BPM;     // Current BPM
static unsigned long beatInterval = 1000;  // Interval between beats in ms (60 BPM default)

// Initialize metronome
void initMetronome() {
  // Set all motor control pins as outputs
  pinMode(COIL_A1, OUTPUT);
  pinMode(COIL_A2, OUTPUT);
  pinMode(COIL_B1, OUTPUT);
  pinMode(COIL_B2, OUTPUT);

  // Initialize all pins to LOW
  digitalWrite(COIL_A1, LOW);
  digitalWrite(COIL_A2, LOW);
  digitalWrite(COIL_B1, LOW);
  digitalWrite(COIL_B2, LOW);

  // Initialize timing
  currentBpm = DEFAULT_BPM;
  beatInterval = 60000 / currentBpm;  // milliseconds per beat
  swingAmplitude = 90;                // Default to full swing

  Serial.println("Stepper motor initialized");

  // Center the metronome
  centerMetronome();

  // Turn off motor coils to prevent overheating when not moving
  disableMotor();
}

// Enable motor coils
void enableMotor() {
  motorEnabled = true;
}

// Disable motor coils to prevent overheating
void disableMotor() {
  digitalWrite(COIL_A1, LOW);
  digitalWrite(COIL_A2, LOW);
  digitalWrite(COIL_B1, LOW);
  digitalWrite(COIL_B2, LOW);
  motorEnabled = false;
}

// Step the motor once in the specified direction
void stepMotor(bool clockwise) {
  // Enable motor if not enabled
  if (!motorEnabled) {
    enableMotor();
  }

  // Update step index based on direction
  if (clockwise) {
    currentStepIndex = (currentStepIndex + 1) % 4;
  } else {
    currentStepIndex = (currentStepIndex + 3) % 4;  // Same as -1 but stays positive
  }

  // Apply the step pattern to the motor pins
  digitalWrite(COIL_A1, stepSequence[currentStepIndex][0]);
  digitalWrite(COIL_A2, stepSequence[currentStepIndex][1]);
  digitalWrite(COIL_B1, stepSequence[currentStepIndex][2]);
  digitalWrite(COIL_B2, stepSequence[currentStepIndex][3]);

  // Update current position
  if (clockwise) {
    currentStepPosition++;
  } else {
    currentStepPosition--;
  }
}

// Convert from degrees to motor steps
int degreesToSteps(int degrees) {
  // Simple conversion from degrees to steps
  return (degrees * STEPS_PER_REVOLUTION) / 360;
}

// Center the metronome
void centerMetronome() {
  Serial.println("Centering metronome...");

  // Enable motor
  enableMotor();

  // First, do a reset sequence to ensure motor is responsive
  // Move a small amount in each direction
  for (int i = 0; i < 20; i++) {
    stepMotor(true);
    delay(5);
  }

  for (int i = 0; i < 20; i++) {
    stepMotor(false);
    delay(5);
  }

  // Reset position counters
  currentStepPosition = 0;
  targetStepPosition = 0;
  currentDegreePosition = 0;
  swingDirectionRight = true;

  Serial.println("Metronome centered");

  // Turn off motor coils to prevent overheating
  disableMotor();
}

// Set BPM with constraints
void setBPM(float bpm) {
  // Constrain BPM to valid range
  bpm = constrain(bpm, StepperMotor::MIN_BPM, StepperMotor::MAX_BPM);

  // Only update if there's a meaningful change
  if (abs(bpm - currentBpm) > 1.0) {
    currentBpm = bpm;
    beatInterval = 60000 / currentBpm;  // milliseconds per beat

    // Adjust swing amplitude based on BPM
    if (currentBpm > BPM_THRESHOLD_3) {
      swingAmplitude = 60;  // At very high BPM, smaller swing
    } else if (currentBpm > BPM_THRESHOLD_2) {
      swingAmplitude = 90;  // At high BPM, reduced swing
    } else if (currentBpm > BPM_THRESHOLD_1) {
      swingAmplitude = 120;  // At medium-high BPM, moderate swing
    } else {
      swingAmplitude = 150;  // At normal BPM, full swing
    }

    Serial.print("BPM updated to: ");
    Serial.print(currentBpm);
    Serial.print(", Swing amplitude: ±");
    Serial.print(swingAmplitude);
    Serial.println("°");
  }
}

// Update metronome based on BPM
void updateMetronome(float bpm) {
  // Update BPM if changed
  setBPM(bpm);

  unsigned long currentTime = millis();

  // Calculate step delay based on BPM
  // Higher BPM = faster steps = smaller delay
  int stepDelay = max(2, (int)(15 - (currentBpm / 20)));
  // Serial.println(currentBpm);
  // Serial.println(stepDelay);
  // int calculatedDelay = (int)(swingAmplitude*currentBpm*25/(3*360/STEPS_PER_REVOLUTION));
  // int stepDelay = max(2,calculatedDelay);

  // Move motor steps if needed
  if (currentTime - lastStepTime >= stepDelay) {
    lastStepTime = currentTime;

    // Check if we need to move to reach target position
    if (currentStepPosition < targetStepPosition) {
      stepMotor(true);
    } else if (currentStepPosition > targetStepPosition) {
      stepMotor(false);
    } else {
      // Target position reached, check if we need to switch direction
      if (currentTime - lastBeatTime >= (beatInterval / 2)) {
        lastBeatTime = currentTime;

        // Toggle direction
        swingDirectionRight = !swingDirectionRight;

        // Calculate new target position
        int targetDegrees = swingDirectionRight ? swingAmplitude : -swingAmplitude;
        targetStepPosition = degreesToSteps(targetDegrees);

        // Update current degrees position
        currentDegreePosition = targetDegrees;

        // Set beat started flag if we're starting a new beat (moving right)
        beatStarted = swingDirectionRight;

        // Serial.print("New beat, direction: ");
        // Serial.println(swingDirectionRight ? "right" : "left");
      } else {
        // We've reached target but not time to change yet, turn off motor
        disableMotor();
        beatStarted = false;
      }
    }
  }
}

// Set metronome to specific position
void setMetronomePosition(int degrees) {
  // Constrain to valid range
  if (degrees < -swingAmplitude) degrees = -swingAmplitude;
  if (degrees > swingAmplitude) degrees = swingAmplitude;

  // Convert degrees to steps
  targetStepPosition = degreesToSteps(degrees);

  // Enable motor before moving
  enableMotor();

  // Update current position tracking
  currentDegreePosition = degrees;
}

// Get current position in degrees
int getMetronomePosition() {
  return currentDegreePosition;
}

// Check if new beat started
bool newBeatStarted() {
  return beatStarted;
}