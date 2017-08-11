#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>

const uint16_t EEPROM_SIZE = 256;

ESP8266WebServer server(80);

/* MQTT Client */
WiFiClient espClient;
PubSubClient mqttClient(espClient);
long lastMqttConnAtt = 0;
uint8_t state = 0;


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.println("Setup started");
  if (existConfig()) {
    Serial.println("Configuration found. Launching module.");
    loadConf();
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
