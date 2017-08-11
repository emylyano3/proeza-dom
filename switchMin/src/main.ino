#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>

ESP8266WebServer server(80);

/* MQTT Client */
WiFiClient espClient;
PubSubClient mqttClient(espClient);
long lastMqttConnAtt = 0;

/* App State */
uint8_t state = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\nSetup started");
  if (existConfig()) {
    Serial.println("Configuration found. Launching module.");
    launchModule();
  } else {
    Serial.println("Configuration not found. Launching AP for setup.");
    startAP();
  }
}

void loop() {
  if (state == 0) {
    server.handleClient();
    return;
  } else if (state == 1) {
    if (!mqttClient.connected()) {
      connectBroker();
    }
    mqttClient.loop();
    return;
  } else {
    Serial.printf("Estado invalido %d\n", state);
    while (1);
  }
}