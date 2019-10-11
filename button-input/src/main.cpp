#include <Arduino.h>
int led = 5;     // LED pin
int button = D0; // push button is connected
int temp = 0; 	 // temporary variable for reading the button pin status

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000);
  while (!Serial){};
  pinMode(led, OUTPUT);   // declare LED as output
  pinMode(button, INPUT); // declare push button as input
}

void loop() {
  temp = digitalRead(button);
  Serial.println(temp);
  if (temp == HIGH) {
    digitalWrite(led, HIGH);
    Serial.println("LED Turned ON");
    delay(100);
  }
  else {
    digitalWrite(led, LOW);
    Serial.println("LED Turned OFF");
    delay(100);
  }
}