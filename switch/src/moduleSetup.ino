static const char FORM_WIFI_SETUP [] =
"<!DOCTYPE html>"
"<html>"
"<body>"
	"<h1>ESP Switch</h1>"
  "<h2>Wifi setup</h2>"
	"<div>"
		"<form method='POST' action='switchsetup'>"
		  "SSID<br/>"
		  "<input type='text' name='ssid' required='required' placeholder='ssid'/>"
		  "<br/><p/>"
		  "Password<br/>"
		  "<input type='password' name='pass' placeholder='********'/>"
		  "<br/><p/>"
		  "<input type='submit' value='Aceptar'/>"
		"</form>"
	"</div>"
"</body>"
"</html>";

static const char FORM_SWITCH_SETUP [] =
"<!DOCTYPE html>"
"<html>"
"<body>"
	"<h1>ESP Switch</h1>"
  "<h2>Module setup</h2>"
	"<div>"
		"<form method='POST' action='setupfinish'>"
		  "Nombre<br/>"
		  "<input type='text' name='swName' placeholder='luz-dorm-frente' required='required' pattern='[\\w-]{5,20}' maxlength=20/>"
		  "<br/><p/>"
		  "<input type='submit' value='Aceptar'/>"
		"</form>"
	"</div>"
"</body>"
"</html>";

static const char PAGE_CONFIG_OK [] =
"<!DOCTYPE html>"
"<html>"
	"<body>"
		"<h1>Configuration OK</h1>"
		"<h2>Server shuting down...</h2>"
	"</body>"
"</html>";

/* Access Point */
const char * APssid = "ESP-AP";
const char * APpass = "12345678";
IPAddress apIP(192, 168, 5, 1);
IPAddress netMsk(255, 255, 255, 0);

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
  Serial.println("Starting Access Point...");
	WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  if (WiFi.softAP(APssid, APpass)) {
    // Without delay I've seen the IP address blank
    delay(500);
    Serial.print("AP Setup Success. IP address: ");
    Serial.println(WiFi.softAPIP());
		webServer.on("/wifisetup", handleWifiSetup);
		webServer.on("/switchsetup", handleSwitchSetup);
		webServer.on("/setupfinish", handleSetupFinish);
		webServer.onNotFound(handleNotFound);
		// Web server start
		webServer.begin();
		Serial.println("HTTP server started");
		return true;
  } else {
    Serial.println("AP Setup Fail.");
		return false;
  }
}

void stopWebServer () {
	webServer.stop();
	WiFi.mode(WIFI_OFF);
	APRunning = false;
}

void handleWifiSetup() {
  webServer.send(200, "text/html", FORM_WIFI_SETUP);
}

void handleSwitchSetup() {
	webServer.arg("ssid").toCharArray(ssid, sizeof(ssid) - 1);
	webServer.arg("pass").toCharArray(pass, sizeof(pass) - 1);
  webServer.send(200, "text/html", FORM_SWITCH_SETUP);
}

void handleSetupFinish () {
  webServer.arg("swName").toCharArray(moduleName, sizeof(moduleName) - 1);
	webServer.send(200, "text/html", PAGE_CONFIG_OK);
	webServer.client().stop();
	delay(500);
	stopWebServer();
	delay(500);
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
    message += " " + webServer.argName ( i ) + ": " + webServer.arg ( i ) + "\n";
  }
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.send(404, "text/html", message);
}
