#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// #include <WiFiClient.h>
// #include <PubSubClient.h>

IPAddress apIP(192, 168, 5, 1);
IPAddress netMsk(255, 255, 255, 0);
ESP8266WebServer server(80);

uint8_t state = 0;

void setup() {
  Serial.begin(115200);
  long aux = millis() + 12000;
  while (aux > millis()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  WiFi.mode(WIFI_AP);
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
    return;
  } else {
    Serial.printf("Estado invalido %d\n", state);
    return;
  }
}

void handleStart () {
  server.send(200, "text/html", "<!DOCTYPE html><html>Starting module</html>");
  server.stop();
  connectStation();
  state = 1;
}

int connectStation() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname("light_switch");
  Serial.println("Connecting station to ssid: dd-wrt-low pass: sabarasa");
  WiFi.begin("dd-wrt-low", "sabarasa");
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

void handleSetup() {
  server.send(200, "text/html", "<!DOCTYPE html><html>Setup Finish<p/><form method='POST' action='start'><input type='submit' value='Iniciar Modulo'/></form></html>");
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
