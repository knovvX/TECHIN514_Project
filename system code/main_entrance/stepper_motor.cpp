#include "stepper_motor.h"

// Stepper motor step sequence (half step mode for smoother movement)
const int stepSequence[8][4] = {
    {1, 0, 0, 0}, // Step 1
    {1, 1, 0, 0}, // Step 2
    {0, 1, 0, 0}, // Step 3
    {0, 1, 1, 0}, // Step 4
    {0, 0, 1, 0}, // Step 5
    {0, 0, 1, 1}, // Step 6
    {0, 0, 0, 1}, // Step 7
    {1, 0, 0, 1}  // Step 8
};

// Metronome configuration
const int STEPS_PER_REVOLUTION = 200;         // Standard for most steppers (1.8° per step)
const int STEPS_FOR_180_DEGREES = STEPS_PER_REVOLUTION / 2; // 100 steps
const int MAX_SWING_STEPS = STEPS_FOR_180_DEGREES / 2; // 50 steps (90 degrees) to each side

// BPM thresholds for adjusting swing amplitude
const float BPM_THRESHOLD_1 = 120.0;  // Above this, reduce swing to 140°
const float BPM_THRESHOLD_2 = 160.0;  // Above this, reduce swing to 100°
const float BPM_THRESHOLD_3 = 200.0;  // Above this, reduce swing to 60°

// Power control for different speeds
const int DEFAULT_POWER = 255;        // Default power level (max)
const int HIGH_SPEED_POWER = 255;     // Power level for high speed movement
const int LOW_SPEED_POWER = 180;      // Power level for low speed movement

// Metronome state
static int currentStepPosition = 0;           // Current physical step position
static int currentDegreePosition = 0;         // Current position in degrees
static bool swingDirectionRight = true;       // Current swing direction
static unsigned long lastBeatTime = 0;        // Last time we processed a beat
static bool beatStarted = false;              // Flag for beat starting
static int swingAmplitude = 90;               // Current swing amplitude (degrees to each side)
static bool motorEnabled = true;              // Flag to enable/disable motor

// For timing calculations
static float currentBpm = 60.0;               // Current BPM
static unsigned long beatInterval = 1000;     // Interval between beats in ms (60 BPM default)

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
    currentBpm = 60.0;
    beatInterval = 60000 / currentBpm; // milliseconds per beat
    swingAmplitude = 90; // Default to full swing
    
    Serial.println("Stepper motor initialized");
    
    // Center the metronome (use a delay to ensure it completes)
    centerMetronome();
    
    // Turn off motor coils to prevent overheating when not moving
    disableMotor();
}

// Move the stepper motor to a specific step position
void moveToStep(int targetStep) {
    // Enable motor before moving
    enableMotor();
    
    // Determine direction
    bool moveClockwise = true;
    
    // Calculate shortest path
    int stepsToMove;
    if (targetStep > currentStepPosition) {
        if (targetStep - currentStepPosition < STEPS_PER_REVOLUTION / 2) {
            moveClockwise = true;
            stepsToMove = targetStep - currentStepPosition;
        } else {
            moveClockwise = false;
            stepsToMove = currentStepPosition + STEPS_PER_REVOLUTION - targetStep;
        }
    } else {
        if (currentStepPosition - targetStep < STEPS_PER_REVOLUTION / 2) {
            moveClockwise = false;
            stepsToMove = currentStepPosition - targetStep;
        } else {
            moveClockwise = true;
            stepsToMove = STEPS_PER_REVOLUTION - currentStepPosition + targetStep;
        }
    }
    
    // Calculate step delay based on BPM
    int stepDelay;
    if (currentBpm > 120.0) {
        // For faster BPM, use shorter delays - fix: ensure integer result
        stepDelay = max(2, (int)(10 - (currentBpm / 30.0)));
    } else {
        // For slower BPM, use longer delays - fix: ensure integer result
        stepDelay = max(3, (int)(15 - (currentBpm / 20.0)));
    }
    
    // Move the motor
    for (int i = 0; i < stepsToMove; i++) {
        // Calculate the next step in the sequence (using 8-step sequence for half-stepping)
        int stepIndex;
        if (moveClockwise) {
            stepIndex = (currentStepPosition % 8);
            currentStepPosition = (currentStepPosition + 1) % STEPS_PER_REVOLUTION;
        } else {
            stepIndex = ((currentStepPosition - 1 + 8) % 8);
            currentStepPosition = (currentStepPosition - 1 + STEPS_PER_REVOLUTION) % STEPS_PER_REVOLUTION;
        }
        
        // Apply the step pattern to the motor pins
        digitalWrite(COIL_A1, stepSequence[stepIndex][0]);
        digitalWrite(COIL_A2, stepSequence[stepIndex][1]);
        digitalWrite(COIL_B1, stepSequence[stepIndex][2]);
        digitalWrite(COIL_B2, stepSequence[stepIndex][3]);
        
        // Short delay for the motor to complete the step
        delay(stepDelay);
    }
}

// Convert from degrees to motor steps
int degreesToSteps(int degrees) {
    // Map degrees to steps
    float scaleFactor = (float)swingAmplitude / 90.0;
    int maxSteps = (int)(MAX_SWING_STEPS * scaleFactor);
    return map(degrees, -swingAmplitude, swingAmplitude, -maxSteps, maxSteps);
}

// Convert from motor steps to degrees
int stepsToDegrees(int steps) {
    // Map steps to degrees
    float scaleFactor = (float)swingAmplitude / 90.0;
    int maxSteps = (int)(MAX_SWING_STEPS * scaleFactor);
    return map(steps, -maxSteps, maxSteps, -swingAmplitude, swingAmplitude);
}

// Center the metronome
void centerMetronome() {
    Serial.println("Centering metronome...");
    
    // Enable motor
    enableMotor();
    
    // First, move a full revolution to ensure the motor is working
    for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
        int stepIndex = i % 8; // Use 8-step sequence
        
        digitalWrite(COIL_A1, stepSequence[stepIndex][0]);
        digitalWrite(COIL_A2, stepSequence[stepIndex][1]);
        digitalWrite(COIL_B1, stepSequence[stepIndex][2]);
        digitalWrite(COIL_B2, stepSequence[stepIndex][3]);
        
        delay(3);
    }
    
    // Now calculate and move to center position
    currentStepPosition = 0;
    currentDegreePosition = 0;
    swingDirectionRight = true;
    
    Serial.println("Metronome centered");
    
    // Turn off motor coils to prevent overheating
    disableMotor();
}

// Adjust swing amplitude based on BPM
void adjustSwingForBPM(float bpm) {
    int oldAmplitude = swingAmplitude;
    
    if (bpm >= BPM_THRESHOLD_3) {
        swingAmplitude = 30; // At very high BPM, only swing 60° total (30° each side)
    } else if (bpm >= BPM_THRESHOLD_2) {
        swingAmplitude = 50; // At high BPM, only swing 100° total (50° each side)
    } else if (bpm >= BPM_THRESHOLD_1) {
        swingAmplitude = 70; // At medium-high BPM, swing 140° total (70° each side)
    } else {
        swingAmplitude = 90; // At normal BPM, full 180° swing (90° each side)
    }
    
    // Only log if amplitude changed
    if (oldAmplitude != swingAmplitude) {
        Serial.print("Adjusted swing amplitude to ±");
        Serial.print(swingAmplitude);
        Serial.println("°");
    }
}

// Update metronome based on BPM
void updateMetronome(float bpm) {
    // Update BPM if changed
    if (abs(bpm - currentBpm) > 1.0) {
        currentBpm = bpm;
        beatInterval = 60000 / currentBpm; // milliseconds per beat
        
        // Adjust swing amplitude based on new BPM
        adjustSwingForBPM(bpm);
        
        Serial.print("BPM: ");
        Serial.print(currentBpm);
        Serial.print(", Swing amplitude: ±");
        Serial.print(swingAmplitude);
        Serial.println("°");
    }
    
    unsigned long currentTime = millis();
    
    // Check if it's time for the next beat
    if (currentTime - lastBeatTime >= (beatInterval / 2)) { // Half-interval for each direction
        lastBeatTime = currentTime;
        
        // Toggle direction
        swingDirectionRight = !swingDirectionRight;
        
        // Calculate target position
        int targetDegrees = swingDirectionRight ? swingAmplitude : -swingAmplitude;
        
        // Set beat started flag if we're starting a new beat (moving right)
        beatStarted = swingDirectionRight;
        
        // Enable motor before moving
        enableMotor();
        
        // Move to new position
        setMetronomePosition(targetDegrees);
        
        // Disable motor after reaching position to prevent overheating
        disableMotor();
    } else {
        // Reset beat started flag
        beatStarted = false;
    }
}

// Set metronome to specific position
void setMetronomePosition(int degrees) {
    // Constrain to valid range
    if (degrees < -swingAmplitude) degrees = -swingAmplitude;
    if (degrees > swingAmplitude) degrees = swingAmplitude;
    
    // Convert degrees to steps
    int targetSteps = degreesToSteps(degrees);
    
    // Move the motor
    moveToStep(targetSteps);
    
    // Update current position
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