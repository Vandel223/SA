/*
 * HC-SR04 Ultrasonic Distance Measurement
 * Sensors and Actuators Laboratory — Ultrasounds
 *
 * Description:
 *   Measures distance to a target using the HC-SR04 ultrasonic module
 *   connected to an Arduino Uno. Uses Pin Change Interrupts (PCINT) for
 *   a non-blocking echo timing approach.
 *
 * Wiring:
 *   HC-SR04 VCC  -> Arduino 5V
 *   HC-SR04 Trig -> Arduino D10
 *   HC-SR04 Echo -> Arduino D9
 *   HC-SR04 GND  -> Arduino GND
 *
 * References:
 *   - "Como Ligar o Sensor Distancia Ultrassom HC-SR04 ao Arduino"[PT],
 *     Arduino Portugal, https://www.arduinoportugal.pt/como-ligar-o-sensor-distancia-ultrassom-hc-sr04-ao-arduino/
 *     (interrupt-based approach adapted from Example 3 — Nível Avançado)
 *   - Code structure and comments by Claude, Anthropic,
 *     claude.ai, 2026
 */

const uint8_t TRIG_PIN = 10;
const uint8_t ECHO_PIN = 9;   // Must be on Port B (D8–D13) for PCINT0

volatile uint32_t pulse_start;   // Timestamp when Echo goes HIGH
volatile uint32_t pulse_time;    // Measured pulse duration (µs)
volatile bool     measurement_ready = false;

uint32_t print_timer = 0;

// ── Setup ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // Enable Pin Change Interrupt on PCINT1 (Arduino D9 = PB1)
  cli();
  PCICR  |= (1 << PCIE0);   // Enable PCINT[7:0] group
  PCMSK0 |= (1 << PCINT1);  // Unmask PCINT1 (D9)
  sei();
}

// ── Interrupt Service Routine ─────────────────────────────────────────────────
// Called on any logic change on D9
ISR(PCINT0_vect) {
  if (PINB & (1 << PINB1)) {
    // Rising edge: Echo went HIGH → record start time
    pulse_start = micros();
  } else {
    // Falling edge: Echo went LOW → compute pulse duration
    pulse_time = micros() - pulse_start;
    measurement_ready = true;
  }
}

// ── Helper: fire one trigger pulse ───────────────────────────────────────────
void fire_trigger() {
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(11);        // ≥ 10 µs required by datasheet
  digitalWrite(TRIG_PIN, LOW);
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void loop() {
  // Print and re-trigger every 500 ms
  if (millis() - print_timer > 500 && measurement_ready) {
    print_timer = millis();

    // Convert pulse width to distance in cm
    // Speed of sound ≈ 343 m/s = 0.0343 cm/µs → one-way = 0.01715 cm/µs
    double distance_cm = 0.01715 * pulse_time;

    Serial.print(distance_cm, 2);
    Serial.println(" cm");

    // Request next measurement
    measurement_ready = false;
    fire_trigger();
  }

  // Kick off the very first measurement
  if (print_timer == 0) {
    fire_trigger();
    print_timer = millis();
  }
}