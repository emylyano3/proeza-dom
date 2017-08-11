
void connectBroker() {
  if (lastMqttConnAtt < millis()) {
    lastMqttConnAtt = millis() + 5000;
    // if (mqttClient.connect(name)) {
    if (mqttClient.connect("switch01")) {
      Serial.println("connected");
      mqttClient.subscribe("light/room01/cmd");
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqttClient.state());
    }
  }
}

int connectStation() {
  Serial.println("Connecting station");
  WiFi.mode(WIFI_STA);
  char value[24] = "";
  readConf("name", value);
  WiFi.hostname(value);
  WiFi.begin("dd-wrt-low", "sabarasa");
  // WiFi.begin(getConfig("/ssid.txt"), pass);
  int status;
  while ((status = WiFi.status()) != WL_CONNECTED) {
    switch (status) {
      case WL_CONNECT_FAILED:
        Serial.println("Could not connect to dd-wrt-low");
        return status;
      case WL_NO_SSID_AVAIL:
        Serial.println("SSID dd-wrt-low is not available");
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

void setBroker () {
  // mqttClient.setServer(mqttBrokerIP, mqttBrokerPort);
  mqttClient.setServer("192.168.0.105", 1883);
  mqttClient.setCallback(callback);
}

void saveSetupForm () {
  String conf = String("|ssid:");
  conf.concat(server.arg("ssid"));
  conf.concat("|pass:");
  conf.concat(server.arg("pass"));
  conf.concat("|doma:");
  conf.concat(server.arg("doma"));
  conf.concat("|name:");
  conf.concat(server.arg("name"));
  conf.concat("|loca:");
  conf.concat(server.arg("loca"));
  conf.concat("|type:");
  conf.concat(server.arg("type"));
  conf.concat("|brIP:");
  conf.concat(server.arg("brIP"));
  conf.concat("|brPO:");
  conf.concat(server.arg("brPO"));
  conf.concat("|");
  char tosave[conf.length()];
  conf.toCharArray(tosave, sizeof(tosave) + 1, 0);
  saveConf(tosave);
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
          "<input type='text' name='doma' placeholder='MiCasa' value='MiCasa' required='required' pattern='[\\w\\-_]{3,15}' maxlength=15/>"
          "<p/><br/>"
          "<b>Nombre</b><br/>"
          "<input type='text' name='name' placeholder='switch01' value='switch01' required='required' pattern='[\\w\\-_]{3,10}' maxlength=10/>"
          "<p/><br/>"
          "<b>Tipo de modulo</b><br/>"
          "<input list='stypes' name='type' value='Light' required='required'/>"
          "<datalist id='stypes'>"
            "<option value='Appliance'/>"
            "<option value='Light'/>"
            "<option value='Power'/>"
            "<option value='Machine'/>"
          "</datalist>"
          "<p/><br/>"
          "<b>Ubicacion</b><br/>"
          "<input type='text' name='loca' placeholder='dorm_01' value='dorm_01' required='required' pattern='[\\w\\-_]{3,15}' maxlength=15/>"
          "<p/><br/>"
          "<b>IP MQTT Broker</b><br/>"
          "<input type='text' name='brIP' value='192.168.0.105' required='required' pattern='[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}' maxlength=15/>"
          "<p/><br/>"
          "<b>Puerto MQTT Broker</b><br/>"
          "<input type='text' name='brPO' value='1883' required='required' pattern='[\\d]{1,5}' maxlength=5/>"
          "<p/><br/>"
          "<input type='submit' value='Iniciar modulo'/>"
        "</form>"
      "</div>"
    "</body>"
    "</html>";
  return String(form);
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

void handleStart () {
  server.send(200, "text/html", getStartingMessage());
  server.stop();
  saveSetupForm();
  launchModule();
}

void launchModule () {
  loadConf();
  connectStation();
  setBroker();
  state = 1;
}

void handleNotFound() {
  // String message = "<h1>File Not Found</h1><br/>";
  // message += "URI: ";
  // message += server.uri();
  // message += "<br/>Method: ";
  // message += (server.method() == HTTP_GET) ? "GET" : "POST";
  // message += "<br/>Arguments: ";
  // message += server.args();
  // message += "<p/>";
  // for ( uint8_t i = 0; i < server.args(); i++ ) {
  //   message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  // }
  // server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  // server.sendHeader("Pragma", "no-cache");
  // server.sendHeader("Expires", "-1");
  server.send(404, "text/html", "<h1>Not found</h1>");
}

void startAP () {
  ESP.wdtFeed();
  WiFi.mode(WIFI_AP);
  IPAddress apIP(192, 168, 5, 1);
  IPAddress netMsk(255, 255, 255, 0);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP("ESP-AP", "12345678");
  Serial.println("AP started");
  server.on("/setup", handleSetup);
  server.on("/start", handleStart);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Server started");
}
