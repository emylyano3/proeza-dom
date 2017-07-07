/*
Codigo para ESP Switch via MQTT

Se implemnta sobre una maquina de estados
Estados posibles
  1. LOAD_COMF
  2. SETUP
  3. SAVE_CONF
  4. RUN

Primer boot
-----------
  1. LOAD_CONF: Se intentan obtener las cedenciales persistidas
    1.1 SETUP: Se entra en este estado si no se logra cargar la configuracion del
    moduloe. El modulo en modo AP y levanta un web server publicando en la IP asingada
    un formulario para el ingreso de la configuracion.
    1.2 SAVE_CONF: Una vez ingresadas las credenciales se persisten
  2. RUN: Finalizada la etapa de configuracion se inicializa el modulo en modo cliente.
  Con la configuracion establecida se conecta al AP de la red y se conecta al broker MQTT.
    2.1 SETUP: Se entra en este estado si Luego de 10 reintentos no se puede establecer
    algunas de las conexiones.

Las transiciones de estado posibles:
  LOAD_CONF --> RUN
  LOAD_CONF --> SETUP
  SETUP ------> SAVE_CONF
  SAVE_CONF --> RUN
  RUN --------> SETUP
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <FS.h>

/* MQTT broker config */
const char* mqttServerIP = "192.168.0.105";
int mqttServerPort = 1883;

/* Wifi Network */
char ssid[32] = "none";
char pass[32] = "none";

/* Module Name */
char moduleName[20] = "none";

/* Module states */
const char STATE_LOAD_CONF  = 0;
const char STATE_SETUP      = 1;
const char STATE_SAVE_CONF  = 2;
const char STATE_RUN        = 3;

const char* STATES[] = {"LOAD_CONF", "SETUP", "SAVE_CONF", "RUN"};

char currentState = STATE_LOAD_CONF;

void setup () {
  Serial.begin(115200);
  delay(5000);
}

void loop () {
  switch (currentState) {
    case STATE_LOAD_CONF:
      loadConfig();
    break;
    case STATE_SETUP:
      moduleSetup();
    break;
    case STATE_RUN:
      moduleRun();
    break;
    case STATE_SAVE_CONF:
      saveConfig();
    break;
    default:
      Serial.printf("Invalid module state %d. Fatal error.", currentState);
      while (true);
  }
}

void setState (char state) {
  if (state <= STATE_RUN) {
    currentState = state;
    Serial.print("State changed to: ");
    Serial.println(STATES[state]);
  } else {
    Serial.printf("Invalid state: %d\n", state);
  }
}
