/*
  DC Motor Speed Measurement with Hall Effect Sensor
  Lab: DC Motor and Magnetic Field Sensor
  
  - Uses hysteresis comparator to detect magnet passes
  - Calculates RPM from pulse timing
  - Cycles through 5 duty cycles (10%, 20%, 30%, 40%, 50%) for transfer function
*/

#define FORWARD 0
#define REVERSE 1
#define MOTOR_A 0
#define MOTOR_B 1

#define DIRA 12
#define PWMA 3
#define DIRB 4
#define PWMB 11
#define ROT A0

// --- Hysteresis Thresholds ---
// Tune these after observing your sensor's raw output range
// (run the test sketch first and note min/max values)
#define THRESH_HIGH 600   // threshold going UP   (magnet approaching)
#define THRESH_LOW  200   // threshold going DOWN  (magnet leaving)

// --- RPM Measurement ---
volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;  // microseconds between pulses
volatile bool newPulse = false;

// Hysteresis state
bool aboveThreshold = false;

// --- Transfer Function Test ---
// Duty cycles as PWM values (0–255): 10%=26, 20%=51, 30%=77, 40%=102, 50%=128
const byte dutyCycles[] = {26, 51, 77, 102, 128};
const int numSteps = 5;
int currentStep = 0;

unsigned long stepStartTime = 0;
const unsigned long SETTLE_TIME   = 3000;  // ms to let motor stabilise
const unsigned long MEASURE_TIME  = 5000;  // ms to average RPM

bool settling = true;

// RPM averaging
float rpmAccumulator = 0;
int   rpmSampleCount = 0;


void setup() {
  Serial.begin(115200);
  setupArdumoto();
  pinMode(ROT, INPUT);

  Serial.println("=== DC Motor Transfer Function Measurement ===");
  Serial.println("Format: step, PWM_value, duty_%, voltage_approx, RPM");
  Serial.println();

  // Start first step
  applyStep(currentStep);
}


void loop() {
  // ── 1. Read sensor and apply hysteresis comparator ──────────────────────
  int sensorVal = analogRead(ROT);
  unsigned long now = micros();

  if (!aboveThreshold && sensorVal > THRESH_HIGH) {
    // Rising edge detected → magnet passed
    aboveThreshold = true;
    if (lastPulseTime != 0) {
      pulseInterval = now - lastPulseTime;
      newPulse = true;
    }
    lastPulseTime = now;
  } else if (aboveThreshold && sensorVal < THRESH_LOW) {
    // Falling edge — reset for next pulse
    aboveThreshold = false;
  }

  // ── 2. Compute RPM from pulse interval ──────────────────────────────────
  float rpm = 0;
  if (newPulse) {
    newPulse = false;
    // pulseInterval is time for ONE full revolution (one magnet on shaft)
    // If you have N magnets, divide by N
    rpm = 60.0e6 / pulseInterval;  // RPM = 60s * 1e6us/s / interval_us
  }

  // ── 3. Transfer function stepping logic ─────────────────────────────────
  unsigned long elapsed = millis() - stepStartTime;

  if (settling) {
    // During settle period: just print raw sensor for debugging
    if (elapsed < SETTLE_TIME) {
      // Optional: stream raw sensor so you can watch it stabilise
      // Serial.print("RAW: "); Serial.println(sensorVal);
    } else {
      // Done settling → start measuring
      settling = false;
      rpmAccumulator = 0;
      rpmSampleCount = 0;
      stepStartTime = millis();
      Serial.print("Measuring step ");
      Serial.print(currentStep + 1);
      Serial.print(" (PWM=");
      Serial.print(dutyCycles[currentStep]);
      Serial.println(")...");
    }

  } else {
    // Measurement window: accumulate RPM samples
    if (rpm > 0) {
      rpmAccumulator += rpm;
      rpmSampleCount++;
    }

    if (elapsed >= MEASURE_TIME) {
      // Print averaged result
      float avgRPM = (rpmSampleCount > 0) ? rpmAccumulator / rpmSampleCount : 0;
      byte  pwmVal  = dutyCycles[currentStep];
      float dutyPct = pwmVal / 255.0 * 100.0;
      // Approximate voltage = duty% * Vsupply (adjust Vsupply to match your bench)
      float Vsupply = 5.0;  // ← change to your DC bench voltage
      float voltage = (dutyPct / 100.0) * Vsupply;

      Serial.print("RESULT | Step: ");   Serial.print(currentStep + 1);
      Serial.print(" | PWM: ");          Serial.print(pwmVal);
      Serial.print(" | Duty: ");         Serial.print(dutyPct, 1); Serial.print("%");
      Serial.print(" | V_approx: ");     Serial.print(voltage, 2); Serial.print("V");
      Serial.print(" | Avg RPM: ");      Serial.println(avgRPM, 1);

      // Advance to next step
      currentStep++;
      if (currentStep < numSteps) {
        applyStep(currentStep);
      } else {
        // All steps done
        stopArdumoto(MOTOR_A);
        Serial.println();
        Serial.println("=== Measurement complete. Motor stopped. ===");
        while (true); // halt
      }
    }
  }

  // ── 4. Live RPM display (always) ────────────────────────────────────────
  if (rpm > 0) {
    Serial.print("Live RPM: ");
    Serial.print(rpm, 1);
    Serial.print("  |  Sensor: ");
    Serial.println(sensorVal);
  }

  delay(5); // small delay to avoid flooding serial
}


// ── Helper: start a new PWM step ────────────────────────────────────────────
void applyStep(int step) {
  driveArdumoto(MOTOR_A, FORWARD, dutyCycles[step]);
  stepStartTime = millis();
  settling = true;
  lastPulseTime = 0;  // reset timing
  Serial.print("\n--- Step ");
  Serial.print(step + 1);
  Serial.print(": PWM = ");
  Serial.print(dutyCycles[step]);
  Serial.println(" | Settling...");
}


// ── Ardumoto helpers ────────────────────────────────────────────────────────
void driveArdumoto(byte motor, byte dir, byte spd) {
  if (motor == MOTOR_A) {
    digitalWrite(DIRA, dir);
    analogWrite(PWMA, spd);
  } else if (motor == MOTOR_B) {
    digitalWrite(DIRB, dir);
    analogWrite(PWMB, spd);
  }
}

void stopArdumoto(byte motor) { driveArdumoto(motor, 0, 0); }

void setupArdumoto() {
  pinMode(PWMA, OUTPUT); pinMode(PWMB, OUTPUT);
  pinMode(DIRA, OUTPUT); pinMode(DIRB, OUTPUT);
  digitalWrite(PWMA, LOW); digitalWrite(PWMB, LOW);
  digitalWrite(DIRA, LOW); digitalWrite(DIRB, LOW);
}