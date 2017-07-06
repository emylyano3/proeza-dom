
const char FORM_WIFI_SETUP [] =
"<!DOCTYPE html>"
"<html>"
"<body>"
	"<h1>ESP Switch</h1>"
  "<h2>Wifi setup</h2>"
	"<div>"
		"<form method='POST' action='wifisave'>"
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

const char FORM_SWITCH_SETUP [] =
"<!DOCTYPE html>"
"<html>"
"<body>"
	"<h1>ESP Switch</h1>"
  "<h2>Module setup</h2>"
	"<div>"
		"<form method='POST' action='switchsave'>"
		  "Nombre<br/>"
		  "<input type='text' name='swName' placeholder='luz-dorm-frente' required='required' pattern='[\\w-]{5,20}' maxlength=20/>"
		  "<br/><p/>"
		  "<input type='submit' value='Aceptar'/>"
		"</form>"
	"</div>"
"</body>"
"</html>";

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  for ( uint8_t i = 0; i < webServer.args(); i++ ) {
    message += " " + webServer.argName ( i ) + ": " + webServer.arg ( i ) + "\n";
  }
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.send ( 404, "text/plain", message );
}

void handleWifiSetup() {
  webServer.send(200, "text/html", FORM_WIFI_SETUP);
}

void handleSwitchSetup() {
  webServer.send(200, "text/html", FORM_SWITCH_SETUP);
}

void handleWifiSave () {
  Serial.println("Saving wifi config");
  webServer.arg("ssid").toCharArray(ssid, sizeof(ssid) - 1);
  webServer.arg("pass").toCharArray(pass, sizeof(pass) - 1);
  webServer.sendHeader("Location", "switchsetup", true);
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  // // Empty content inhibits Content-length header so we have to close the socket ourselves.
  webServer.send(302, "text/plain", "");
  // Stop is needed because we sent no content length
  webServer.client().stop();
}

void handleSwitchSave () {
	Serial.println("Saving switch config");
  webServer.arg("swName").toCharArray(moduleName, sizeof(moduleName) - 1);
	saveConfig();
	// Request WLAN connect with new credentials if there is a SSID
	// connect = strlen(ssid) > 0;
}
