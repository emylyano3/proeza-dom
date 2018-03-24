 /*
  Created by Igor Jarc
 See http://iot-playground.com for details
 Please use community forum on website do not contact author directly
 
 External libraries:
 - https://github.com/adamvr/arduino-base64
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */
#include <ESP8266WiFi.h>
#include <Base64.h>

//AP definitions
#define AP_SSID "your AP SSID"
#define AP_PASSWORD "your AP password"

// EasyIoT server definitions
#define EIOT_USERNAME    "admin"
#define EIOT_PASSWORD    "test"
#define EIOT_IP_ADDRESS  "192.168.1.5"
#define EIOT_PORT        80
#define EIOT_NODE        "N8S0"

#define INPUT_PIN         2
#define USER_PWD_LEN      40


char unameenc[USER_PWD_LEN];
bool oldInputState;


void setup() {
  Serial.begin(115200);
  pinMode(INPUT_PIN, INPUT);
  
 
}

void loop() {
  int inputState = digitalRead(INPUT_PIN);;  
  Serial.print("Read: ");
  Serial.println(inputState);
  delay(1000);
}
