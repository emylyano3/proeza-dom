#include <Arduino.h>

#define CONTROL_PIN D3
#define READ_PIN D4

int lastRead = 999;

void setup() {
    Serial.begin(115200);
    // initialize LED digital pin as an output.
    pinMode(CONTROL_PIN, OUTPUT);
    pinMode(READ_PIN, INPUT);
}

void loop() {
//   // turn the LED on (HIGH is the voltage level)
//   digitalWrite(CONTROL_PIN, HIGH);
//   // wait for a second
//   delay(1000);
//   // turn the LED off by making the voltage LOW
//   digitalWrite(CONTROL_PIN, LOW);
//    // wait for a second
//   delay(1000);
  int read = digitalRead(READ_PIN);
  if (lastRead != read) {
    Serial.println("Read something diff");
    lastRead = read;
    if (read == LOW) {
        Serial.println("Read LOW");
    } else if (read == HIGH) {
        Serial.println("Read HIGH");
    } else {
        Serial.print("Read: ");
        Serial.println(read);
    }
  }
}