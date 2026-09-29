#include <DS18B20.h>

#define DS_RES  10

DS18B20 ds(2);

const unsigned long INTERVAL = 500;   // desired period in ms
unsigned long lastSampleTime = 0;

/*************************************************************
 *  [Helper Function]
 *  printTimestamp
 *
 *  @description
 *  Prints the timestamp in the format <min>:<sec>.<ms>
 *
 *  @param - ms: timestamp in ms to print
 *************************************************************/
void printTimestamp(unsigned long ms) {
  unsigned long s  = ms / 1000;
  ms %= 1000;
  if (s  < 10) Serial.print('0'); Serial.print(s);  Serial.print('.');
  if (ms < 100) Serial.print('0');
  if (ms < 10)  Serial.print('0');
  Serial.print(ms);
}

void setup() {
  Serial.begin(115200);
  while (ds.selectNext()) ds.setResolution(DS_RES); // Set resolution
}

void loop() {
  unsigned long now = millis();

  if (now - lastSampleTime >= INTERVAL) {

    printTimestamp(now); Serial.print(", ");

    int index = 0;
    while (ds.selectNext()) {
      if (index != 0) Serial.print(", ");
      Serial.print(ds.getTempC(), 3);
      index++;
    }

    if (index == 0) {
      Serial.println("  No sensors found.");
    }

    Serial.println();

    lastSampleTime = now;
  }

  // free to do other work here without blocking
}
