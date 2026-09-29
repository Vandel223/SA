// ============================================================
//  Task 1 — LED Blink: 3 seconds ON, 3 seconds OFF
//  Arduino Uno | Pin 13 → 150 Ω resistor → Red LED → GND
// ============================================================

const int LED_PIN      = 13;   // Digital output pin
const int ON_TIME_MS   = 5000; // ON  duration in milliseconds
const int OFF_TIME_MS  = 5000; // OFF duration in milliseconds

void setup() {
  pinMode(LED_PIN, OUTPUT);    // Configure pin 13 as output
  digitalWrite(LED_PIN, HIGH); // Turn LED ON
}

void loop() {
  // nothing
}