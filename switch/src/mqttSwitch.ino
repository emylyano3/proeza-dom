/*
Codigo para ESP Switch via MQTT

Se implemnta sobre una maquina de estados
Estados posibles
  1. LOAD_CREDENTIALS
  2. WIFI_CONFIG
  3. SAVE_CREDENTIALS
  4. CONNECT_WIFI
  5. CONNECT_BROKER
  6. RUN

Primer boot
-----------
  1. LOAD_CREDENTIALS: Se intentan obtener las cedenciales persistidas en la EEPROM
    1.1 WIFI_CONFIG: Se entra en este estado si no se logran cargar las credenciales. El modulo
    entra en modo server publicando en la IP asingada un formulario para el ingreso de las credenciales
    necesarias para el funcionamiento del modulo
    1.2 SAVE_CREDENTIALS: Una vez ingresadas las credenciales se persisten en la EEPROM
    1.3 LOAD_CREDENTIALS: Se cargan las credenciales
  2. CONNECT_WIFI: Se conecta al wifi configurado. Se reintenta la conexion una cantidad N de veces
  (a definir) Si no se puede realizar la conexion luego de los reintentos se pasa nuevamente al estado
  LOAD_CREDENTIALS
  3. CONNECT_BROKER: Luego de conectar al wifi se realiza la conexion con el broker. Se reintenta una
  cantidad M de veces. Si no se puede realizar la conexion luego de los reintentos se pasa nuevamente al estado
  LOAD_CREDENTIALS
  4. RUN: FInalmente se entra en el estado de ejecucion normal (logica del switch)
  5. CONFIG_SWITCH: Estado en el que se pueden configurar ciertos aspectos del switch.
  E.g.: IP (estatica) del switch, Nombre del switch
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h>

/* MQTT Client */
WiFiClient espClient;
PubSubClient mqttClient(espClient);

/* MQTT broker config */
const char* mqtt_server = "192.168.0.105";

/* Topics config */
const char* TOPIC_COMMAND = "light/room01/cmd";

/* Access Point */
const char * APssid = "ESP-AP";
const char * APpass = "12345678";
IPAddress apIP(192, 168, 5, 1);
IPAddress netMsk(255, 255, 255, 0);

/* Web server para manejar la config */
ESP8266WebServer webServer(80);

/* Wifi Network */
char ssid[32] = "none";
char pass[32] = "none";

/* Module Name */
char moduleName[20] = "ESP_Switch";

/* Module states */
const char STATE_LOAD_CONF  = 0;
const char STATE_SETUP      = 1;
const char STATE_SAVE_CONF  = 2;
const char STATE_RUN        = 3;

const char* STATES[] = {"LOAD_CONF", "SETUP", "SAVE_CONF", "RUN"};

/* Control flags */
boolean AP_RUNNING = false;
boolean CLIENT_RUNNING = false;

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
      Serial.printf("Invalid module mode %c. Fatal error.", currentState);
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

void moduleRun () {
  if (!mqttClient.connected()) {
    connectBroker();
  }
  mqttClient.loop();
}

void connectBroker() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(moduleName)) {
      Serial.println("connected");
      // once connected subscribe
      mqttClient.subscribe(TOPIC_COMMAND);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
