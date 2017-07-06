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

/* IP del AP */
IPAddress apIP(192, 168, 5, 1);
IPAddress netMsk(255, 255, 255, 0);

/* Web server para manejar la config */
ESP8266WebServer webServer(80);

/* MQTT Client */
WiFiClient espClient;
PubSubClient mqttClient(espClient);

/* Access Point */
const char * APssid = "ESP-AP";
const char * APpass = "12345678";

/* Wifi Network */
char ssid[32] = "none";
char pass[32] = "none";

/* Module Name */
char moduleName[20] = "ESP_Switch";

/* Module modes */
const char MODE_AP = 0;
const char MODE_CLIENT = 1;

char currentMode = MODE_AP;

void setup () {
  Serial.begin(115200);
  Serial.println("Setup");
  // testCredentialsStore();
  startAP();
  configServer();
}

long lastAPClientsCheck = 0;

void loop () {
  switch (currentMode) {
    case MODE_AP:
      if (lastAPClientsCheck < millis()) {
        lastAPClientsCheck = millis() + 5000;
        Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
      }
      webServer.handleClient();
    break;
    case MODE_CLIENT:
      if (!mqttClient.connected()) {
        // connectBroker();
      }
      mqttClient.loop();
    break;
    default:
      Serial.printf("Invalid module mode %c. Fatal error.", currentMode);
      while (true);
  }
}

void startAP () {
  Serial.println("Starting Access Point...");
  WiFi.softAPConfig(apIP, apIP, netMsk);
  if (WiFi.softAP(APssid, APpass)) {
    // Without delay I've seen the IP address blank
    delay(500);
    Serial.print("AP Setup Success. IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("AP Setup Fail.");
  }
}

void configServer () {
  webServer.on("/wifisetup", handleWifiSetup);
  webServer.on("/wifisave", handleWifiSave);
  webServer.on("/switchsetup", handleSwitchSetup);
  webServer.on("/switchsave", handleSwitchSave);
  webServer.onNotFound(handleNotFound);
  // Web server start
  webServer.begin();
  Serial.println("HTTP server started");
}
