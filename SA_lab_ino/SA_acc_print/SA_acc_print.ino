/*
 * ADXL330 Accelerometer - Serial Plotter
 * Reads X, Y, Z analog outputs and prints them tab-separated
 * for use with the Arduino IDE Serial Plotter (>= 1.6.6).
 *
 * Circuit:
 *   A0 -> ST (self-test, unused)
 *   A1 -> Z out
 *   A2 -> Y out
 *   A3 -> X out
 *   A4 -> GND of module
 *   A5 -> VCC of module
 */

const int groundpin = 18;  // A4 - used as GND for module
const int powerpin  = 19;  // A5 - used as VCC for module
const int xpin = A3;
const int ypin = A2;
const int zpin = A1;

void setup() {
  Serial.begin(115200);

  // Power the module from analog pins used as digital I/O
  pinMode(groundpin, OUTPUT);
  pinMode(powerpin,  OUTPUT);
  digitalWrite(groundpin, LOW);
  digitalWrite(powerpin,  HIGH);
}

void loop() {
  Serial.print(analogRead(xpin));   // X value
  Serial.print(", ");
  Serial.print(analogRead(ypin));   // Y value
  Serial.print(", ");
  Serial.println(analogRead(zpin)); // Z value (println ends the line)

  delay(100);  // ~10 samples/second
}