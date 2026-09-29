// ============================================================
//  Task 2 — Photoresistor Module Analogue Reading
//  Arduino Uno | AO pin → A0 | Read every 500 ms → Serial
// ============================================================

const int LDR_AO_PIN   = A0;    // Analogue input from LDR module AO
const int SAMPLE_MS    = 500;   // Sampling period: 500 ms
const float VCC        = 5.0;   // Arduino reference voltage (V)
const int ADC_MAX      = 1023;  // 10-bit ADC full-scale

void setup() {
  Serial.begin(9600);           // Open serial at 9600 baud
  // A0 is INPUT by default — no pinMode needed for analogRead
  Serial.println("Time(ms), ADC_raw, Voltage(V)");
}

void loop() {
  int   rawValue  = analogRead(LDR_AO_PIN);
  // Convert 10-bit ADC reading to voltage:
  // V = raw × (VCC / ADC_MAX)
  float voltage   = rawValue * (VCC / ADC_MAX);

  // Print timestamp, raw ADC value, and computed voltage
  Serial.print(millis());
  Serial.print(", ");
  Serial.print(rawValue);
  Serial.print(", ");
  Serial.println(voltage, 3);   // 3 decimal places

  delay(SAMPLE_MS);
}