#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <FS.h>

ESP8266WebServer server(80);

/* Wifi Network config */
char ssid[32] = "";
char pass[32] = "";

/* Module config */
char name[20]     = "";
char domain[20]   = "";
char type[20]     = "";
char location[20] = "";

char mqttBrokerIP[16] = "";
int mqttBrokerPort = 0;

/* MQTT Client */
WiFiClient espClient;
PubSubClient mqttClient(espClient);
long lastMqttConnAtt = 0;

uint8_t state = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Setup started");
  initFS();
  startAP();
}

void initFS () {
  uint32_t realSize = ESP.getFlashChipRealSize();
  uint32_t ideSize = ESP.getFlashChipSize();
  if (realSize == ideSize) {
    Serial.print("Flash correctly configured. SPIFFS opened: ");
    Serial.println(SPIFFS.begin());
  } else {
    Serial.printf("Flash incorrectly configured, cannot start module. IDE size: %d, real size: %d", ideSize, realSize);
    while (1);
  }
}

void startAP () {
  WiFi.mode(WIFI_AP);
  IPAddress apIP(192, 168, 5, 1);
  IPAddress netMsk(255, 255, 255, 0);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP("ESP-AP", "12345678");
  server.on("/setup", handleSetup);
  server.on("/start", handleStart);
  server.onNotFound(handleNotFound);
  server.begin();
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

void connectBroker() {
  if (lastMqttConnAtt < millis()) {
    lastMqttConnAtt = millis() + 5000;
    if (mqttClient.connect(name)) {
      Serial.println("connected");
      mqttClient.subscribe("light/room01/cmd");
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqttClient.state());
    }
  }
}

int connectStation() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname(name);
  Serial.println("Connecting station");
  WiFi.begin(ssid, pass);
  int status;
  while ((status = WiFi.status()) != WL_CONNECTED) {
    switch (status) {
      case WL_CONNECT_FAILED:
      case WL_NO_SSID_AVAIL:
        Serial.println("Abort! Wrong configuration.");
        return status;
      case WL_IDLE_STATUS:
      default:
        break;
    }
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  return status;
}

void callback(char* topic, unsigned char* payload, unsigned int length) {
  Serial.print("Message received. ");
  Serial.print("Topic: ");
  Serial.println(topic);
}

void handleStart () {
  server.send(200, "text/html", getStartingMessage());
  server.stop();
  processSetupForm();
  connectStation();
  setBroker();
  state = 1;
}

void setBroker () {
  mqttClient.setServer(mqttBrokerIP, mqttBrokerPort);
  mqttClient.setCallback(callback);
}

void processSetupForm () {
  server.arg("ssid").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("pass").toCharArray(pass, sizeof(pass) - 1);
  server.arg("mDom").toCharArray(domain, sizeof(domain) - 1);
	server.arg("mNam").toCharArray(name, sizeof(name) - 1);
	server.arg("mLoc").toCharArray(location, sizeof(location) - 1);
	server.arg("mTyp").toCharArray(type, sizeof(type) - 1);
	server.arg("mqbrIP").toCharArray(mqttBrokerIP, sizeof(mqttBrokerIP) - 1);
	char aux[6];
	server.arg("mqbrPO").toCharArray(aux, sizeof(aux) - 1);
  mqttBrokerPort = String(aux).toInt();
}

void handleSetup() {
  server.send(200, "text/html", getSetupForm());
}

String getStartingMessage () {
  char sm[] =
    "<!DOCTYPE html>"
    "<html>"
      "<h1>Starting module</h1>"
    "</html>";
    return String(sm);
}

String getSetupForm () {
  char form[] =
  "<!DOCTYPE html>"
  "<html>"
    "<body>"
      "<h1>ESP Module Setup</h1>"
      "<div>"
        "<form method='POST' action='start'>"
          "SSID<br/>"
  			  "<input type='text' name='ssid' required='required' placeholder='ssid'/>"
  			  "<br/><p/>"
  			  "Password<br/>"
  			  "<input type='password' name='pass' placeholder='********'/>"
  			  "<br/><p/>"
          "<b>Dominio</b><br/>"
          "<input type='text' name='mDom' placeholder='MiCasa' value='MiCasa' required='required' pattern='[\\w\\-_]{3,15}' maxlength=15/>"
          "<p/><br/>"
          "<b>Nombre</b><br/>"
          "<input type='text' name='mNam' placeholder='switch01' value='switch01' required='required' pattern='[\\w\\-_]{3,10}' maxlength=10/>"
          "<p/><br/>"
          "<b>Tipo de modulo</b><br/>"
          "<input list='stypes' name='mTyp' value='Light' required='required'/>"
          "<datalist id='stypes'>"
            "<option value='Appliance'/>"
            "<option value='Light'/>"
            "<option value='Power'/>"
            "<option value='Machine'/>"
          "</datalist>"
          "<p/><br/>"
          "<b>Ubicacion</b><br/>"
          "<input type='text' name='mLoc' placeholder='dorm_01' value='dorm_01' required='required' pattern='[\\w\\-_]{3,15}' maxlength=15/>"
          "<p/><br/>"
          "<b>IP MQTT Broker</b><br/>"
          "<input type='text' name='mqbrIP' value='192.168.0.105' required='required' pattern='[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}' maxlength=15/>"
          "<p/><br/>"
          "<b>Puerto MQTT Broker</b><br/>"
          "<input type='text' name='mqbrPO' value='1883' required='required' pattern='[\\d]{1,5}' maxlength=5/>"
          "<p/><br/>"
          "<input type='submit' value='Iniciar modulo'/>"
        "</form>"
      "</div>"
    "</body>"
    "</html>";
  return String(form);
}

void handleNotFound() {
  String message = "<h1>File Not Found</h1><br/>";
  message += "URI: ";
  message += server.uri();
  message += "<br/>Method: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "<br/>Arguments: ";
  message += server.args();
  message += "<p/>";
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/html", message);
}
