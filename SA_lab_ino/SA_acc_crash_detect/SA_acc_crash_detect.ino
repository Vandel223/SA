/*
 * ADXL330 - Crash / Impact Detection
 * Lights the onboard LED (pin 13) when a sudden acceleration
 * spike exceeding CRASH_THRESHOLD is detected on any axis.
 *
 * Same wiring as Task 2.
 */

const int groundpin = 18;
const int powerpin  = 19;
const int xpin = A3;
const int ypin = A2;
const int zpin = A1;
const int ledPin = 13;       // Built-in LED on Arduino Uno

// Threshold: ADC units above baseline that count as a crash.
// ~1 g corresponds to roughly 68 ADC units for the ADXL330 at 3.3V.
// 200 units ≈ ~3g spike -- adjust as needed during testing.
const int CRASH_THRESHOLD = 200;

// How long the LED stays on after a crash (ms)
const unsigned long LED_ON_TIME = 3000;

int baseline_x, baseline_y, baseline_z;
bool crashed = false;
unsigned long crashTime = 0;

void setup() {
  Serial.begin(115200);

  pinMode(groundpin, OUTPUT);
  pinMode(powerpin,  OUTPUT);
  digitalWrite(groundpin, LOW);
  digitalWrite(powerpin,  HIGH);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Calibrate: take baseline readings at rest
  delay(500);  // let power stabilise
  baseline_x = analogRead(xpin);
  baseline_y = analogRead(ypin);
  baseline_z = analogRead(zpin);

  Serial.println("Baseline captured. Ready for crash detection.");
}

void loop() {
  int x = analogRead(xpin);
  int y = analogRead(ypin);
  int z = analogRead(zpin);

  // Deviation from baseline on each axis
  int dx = abs(x - baseline_x);
  int dy = abs(y - baseline_y);
  int dz = abs(z - baseline_z);

  // Calculate impact as the Euclidean distance
  int impact = sqrt(pow(dx, 2) + pow(dy, 2) + pow(z, 2));

  Serial.print(x); Serial.print("\t");
  Serial.print(y); Serial.print("\t");
  Serial.print(z); Serial.print("\t");
  Serial.println(impact);

  if (!crashed && impact > CRASH_THRESHOLD) {
    crashed = true;
    crashTime = millis();
    digitalWrite(ledPin, HIGH);
    Serial.println("*** CRASH DETECTED ***");
  }

  // Turn off LED after LED_ON_TIME ms
  if (crashed && (millis() - crashTime > LED_ON_TIME)) {
    crashed = false;
    digitalWrite(ledPin, LOW);
    Serial.println("System reset.");
  }

  delay(10);  // 100 Hz sampling
}