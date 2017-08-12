
/* Web server para manejar la config */
ESP8266WebServer webServer(80);

bool APRunning = false;

void moduleSetup () {
	if (!APRunning) {
		APRunning = startWebServer();
	}
	webServer.handleClient();
}

bool startWebServer () {
  Serial.println("Starting Access Point for setup...");
	WiFi.mode(WIFI_AP);
	IPAddress apIP(192, 168, 5, 1);
	IPAddress netMsk(255, 255, 255, 0);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  if (WiFi.softAP("ESP-AP", "12345678")) {
    // Without delay I've seen the IP address blank
    delay(500);
    Serial.print("AP Setup Success. IP address: ");
		Serial.println(WiFi.softAPIP());
		webServer.on("/setup", handleSetup);
		webServer.on("/start", handleStart);
		webServer.onNotFound(handleNotFound);
		webServer.begin();
		Serial.println("HTTP server started");
		return true;
  } else {
    Serial.println("AP Setup Fail.");
		return false;
  }
}

void handleSetup() {
  webServer.send(200, "text/html", getSetupForm());
}

void handleStart () {
	webServer.send(200, "text/html", getStartingMessage());
	loadSetupForm();
	stopWebServer();
	setState(STATE_SAVE_CONF);
}

void handleNotFound() {
  String message = "<h1>File Not Found</h1><br/>";
  message += "URI: ";
  message += webServer.uri();
  message += "<br/>Method: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "<br/>Arguments: ";
  message += webServer.args();
  message += "<p/>";
  for ( uint8_t i = 0; i < webServer.args(); i++ ) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.send(404, "text/html", message);
}

void stopWebServer () {
	webServer.stop();
	APRunning = false;
}

void loadSetupForm () {
	String aux;
	aux = webServer.arg("ssid");
	aux.toCharArray(ssid, 32);
	aux = webServer.arg("pass");
	aux.toCharArray(pass, 32);
  // String conf = String("|ssid:");
  // conf.concat(webServer.arg("ssid"));
  // conf.concat("|pass:");
  // conf.concat(webServer.arg("pass"));
  // conf.concat("|doma:");
  // conf.concat(webServer.arg("doma"));
  // conf.concat("|name:");
  // conf.concat(webServer.arg("name"));
  // conf.concat("|loca:");
  // conf.concat(webServer.arg("loca"));
  // conf.concat("|type:");
  // conf.concat(webServer.arg("type"));
  // conf.concat("|brIP:");
  // conf.concat(webServer.arg("brIP"));
  // conf.concat("|brPO:");
  // conf.concat(webServer.arg("brPO"));
  // conf.concat("|");
  // char tosave[conf.length()];
  // conf.toCharArray(tosave, sizeof(tosave) + 1, 0);
  // saveConf(tosave);
}

const char* getSetupForm () {
	// El formulario debe validar todos los campos como obligatorios
	return
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
}

const char* getStartingMessage () {
	return
		"<!DOCTYPE html>"
		"<html>"
			"<h1>Starting module</h1>"
		"</html>";
}