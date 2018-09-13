#include <FS.h>
#include <Arduino.h>
#include <ESP8266WiFi.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiClient.h>

extern "C" {
    #include "user_interface.h"
}

const char HTTP_HEAD[] PROGMEM                      = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM                     = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_SCRIPT[] PROGMEM                    = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM                  = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_ITEM[] PROGMEM                      = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM                = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID' required><br/><input id='p' name='p' length=64 type='password' placeholder='password' required><br/>";
const char HTTP_FORM_INPUT[] PROGMEM                = "<br/><input id='{i}' name='{n}' maxlength={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_FORM_INPUT_LIST[] PROGMEM           = "<br/><input id='{i}' name='{n}' placeholder='{p}' list='d'><datalist id='d'{o}</datalist>";
const char HTTP_FORM_INPUT_LIST_OPTION[] PROGMEM    = "<option>{o}</option>";
const char HTTP_FORM_END[] PROGMEM                  = "<br/><button type='submit'>save</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM                 = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_SAVED[] PROGMEM                     = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
const char HTTP_END[] PROGMEM                       = "</div></body></html>";

// DNS server
const uint8_t   DNS_PORT = 53;

template <class T> void log (T text) {
    if (LOGGING) {
        Serial.print("*SW: ");
        Serial.println(text);
    }
}

template <class T, class U> void log (T key, U value) {
    if (LOGGING) {
        Serial.print("*SW: ");
        Serial.print(key);
        Serial.print(": ");
        Serial.println(value);
    }
}

enum ControlTypes {Combo, Text};

struct ConfigParam {
    const char*         name;       // identificador
    const char*         label;      // legible por usuario
    char*               value;      // valor default
    uint8_t             maxLength;  // longitud limite
    ControlTypes        type;       // tipo de control en formularion
    const char*         customHTML; // html custom
    std::vector<char*>  options;    // optciones para el combo
};

std::unique_ptr<ESP8266WebServer>  _server;
std::unique_ptr<DNSServer>         _dnsServer;

bool            _connect;
const char*     _ssid               = "";
const char*     _pass               = "";
uint8_t         _lastParam          = 0;
const char*     _apPass             = "12345678";
const char*     _customHeadElement  = "";
ConfigParam     _params[MAX_PARAMS];

#ifdef WIFI_MIN_QUALITY
const uint8_t   _minimumQuality = WIFI_MIN_QUALITY;
#else
const uint8_t   _minimumQuality = -1;
#endif

void setup() {
    Serial.begin(115200);
    connectWifiNetwork(loadConfig());
    Serial.printf("Connected to local network %s", WiFi.localIP().toString().c_str());
}

void loop() {
}

void connectWifiNetwork (bool existsConfig) {
    log(F("Connecting to wifi network"));
    bool connected = false;
    while (!connected) {
        if (existsConfig) {
            log(F("Connecting to saved network"));
            if ((connected = !(connectWiFi() != WL_CONNECTED))) {
                log(F("Could not connect to saved network. Going into config mode."));
                connected = startConfigPortal();
            }
        } else {
            log(F("Going into config mode cause no config was found"));
            connected = startConfigPortal();
        }
    }
}

uint8_t connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.hostname(getStationName());
    if (WiFi.SSID()) {
        log("Using last saved values, should be faster");
        //trying to fix connection in progress hanging
        ETS_UART_INTR_DISABLE();
        wifi_station_disconnect();
        ETS_UART_INTR_ENABLE();
        WiFi.begin();
        return waitForConnectResult();
    } else {
        log("No saved credentials");
        return WL_CONNECT_FAILED;
    }
}

bool startConfigPortal() {
    WiFi.mode(WIFI_AP_STA);
    _connect = false;
    setupConfigPortal();
    while(1) {
        // if(configPortalHasTimeout()) break;
        _dnsServer->processNextRequest();
        _server->handleClient();
        if (_connect) {
            _connect = false;
            delay(2000);
            log(F("Connecting to new AP"));
            // using user-provided  _ssid, _pass in place of system-stored ssid and pass
            if (connectWifi(_ssid, _pass) != WL_CONNECTED) {
                log(F("Failed to connect."));
            } else {
                //connected
                WiFi.mode(WIFI_STA);
                //notify that configuration has changed and any optional parameters should be saved
                saveConfig();
                break;
            }
        }
        yield();
    }
    _server.reset();
    _dnsServer.reset();
    return  WiFi.status() == WL_CONNECTED;
}

uint8_t connectWifi(String ssid, String pass) {
    log(F("Connecting as wifi client..."));
    if (WiFi.status() == WL_CONNECTED) {
        log("Already connected. Bailing out.");
        return WL_CONNECTED;
    }
    WiFi.hostname(getStationName());
    WiFi.begin(ssid.c_str(), pass.c_str());
    return waitForConnectResult();
}

uint8_t waitForConnectResult() {
    if (CONNECTION_TIMEOUT == 0) {
        return WiFi.waitForConnectResult();
    } else {
        log(F("Waiting for connection result with time out"));
        unsigned long start = millis();
        boolean keepConnecting = true;
        uint8_t status, retry = 0;
        while (keepConnecting) {
            status = WiFi.status();
            if (millis() > start + CONNECTION_TIMEOUT) {
                keepConnecting = false;
                log(F("Connection timed out"));
            }
            if (status == WL_CONNECTED) {
                keepConnecting = false;
            } else if (status == WL_CONNECT_FAILED) {
                log(F("Connection failed. Retrying: "));
                log(++retry);
                log("Trying to begin connection again");
                WiFi.begin();
            }
            delay(100);
        }
        return status;
    }
}

void setupConfigPortal() {
    _server.reset(new ESP8266WebServer(80));
    _dnsServer.reset(new DNSServer());
    // _configPortalStart = millis();
    String id = String(ESP.getChipId());
    const char* apName = id.c_str();
    log(F("Configuring access point... "), apName);
    if (_apPass != NULL) {
        if (strlen(_apPass) < 8 || strlen(_apPass) > 63) {
            log(F("Invalid AccessPoint password. Ignoring"));
            _apPass = NULL;
        }
        log(_apPass);
    }
    if (_apPass != NULL) {
        WiFi.softAP(apName, _apPass);
    } else {
        WiFi.softAP(apName);
    }
    delay(500); // Without delay I've seen the IP address blank
    log(F("AP IP address: "));
    log(WiFi.softAPIP());

    /* Setup the DNS server redirecting all the domains to the apIP */
    _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

    /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
    _server->on("/", std::bind(handleWifi, false));
    //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    _server->on("/fwlink", std::bind(handleWifi, false)); 
    _server->on("/wifi", std::bind(handleWifi, true)); 
    _server->on("/wifisave", handleWifiSave);
    // _server->on("/r", handleReset);
    _server->onNotFound (handleNotFound);
    _server->begin();
    log(F("HTTP server started"));
}

void handleWifi(bool scan) {
    // If captive portal redirect instead of displaying the page.
    if (captivePortal()) { 
        return;
    }
    String page = FPSTR(HTTP_HEAD);
    page.replace("{v}", "Config ESP");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);
    if (scan) {
        int n = WiFi.scanNetworks();
        log(F("Scan done"));
        if (n == 0) {
            log(F("No networks found"));
            page += F("No networks found. Refresh to scan again.");
        } else {
            //sort networks
            int indices[n];
            for (int i = 0; i < n; i++) {
                indices[i] = i;
            }
            // old sort
            for (int i = 0; i < n; i++) {
                for (int j = i + 1; j < n; j++) {
                    if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
                        std::swap(indices[i], indices[j]);
                    }
                }
            }
            // remove duplicates ( must be RSSI sorted )
            // if (_removeDuplicateAPs) {
            //     String cssid;
            //     for (int i = 0; i < n; i++) {
            //         if (indices[i] == -1) continue;
            //         cssid = WiFi.SSID(indices[i]);
            //         for (int j = i + 1; j < n; j++) {
            //             if (cssid == WiFi.SSID(indices[j])) {
            //                 log("DUP AP: " + WiFi.SSID(indices[j]));
            //                 indices[j] = -1; // set dup aps to index -1
            //             }
            //         }
            //     }
            // }
            //display networks in page
            for (int i = 0; i < n; i++) {
                if (indices[i] == -1) continue; // skip dups
                log(WiFi.SSID(indices[i]));
                log(WiFi.RSSI(indices[i]));
                int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));
                if (_minimumQuality == -1 || _minimumQuality < quality) {
                    String item = FPSTR(HTTP_ITEM);
                    String rssiQ;
                    rssiQ += quality;
                    item.replace("{v}", WiFi.SSID(indices[i]));
                    item.replace("{r}", rssiQ);
                    if (WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE) {
                        item.replace("{i}", "l");
                    } else {
                        item.replace("{i}", "");
                    }
                    //log(item);
                    page += item;
                    delay(0);
                } else {
                    log(F("Skipping due to quality"));
                }
            }
            page += "<br/>";
        }
    }
    page += FPSTR(HTTP_FORM_START);
    char parLength[5];
    // add the extra parameters to the form
    for (int i = 0; i < _lastParam; i++) {
        if (_params[i].name != NULL) {
            if (_params[i].type == Combo) {
                String pitem = FPSTR(HTTP_FORM_INPUT_LIST);
                pitem.replace("{i}", _params[i].name);
                pitem.replace("{n}", _params[i].name);
                String ops = "";
                for (size_t j = 0; j < _params[i].options.size(); ++j) {
                    String op = FPSTR(HTTP_FORM_INPUT_LIST_OPTION);
                    op.replace("{o}", _params[i].options[j]);
                    ops.concat(op);
                }
                pitem.replace("{o}", ops);
                pitem.replace("{c}", _params[i].customHTML);
                page += pitem;
            } else {
                String pitem = FPSTR(HTTP_FORM_INPUT);
                pitem.replace("{i}", _params[i].name);
                pitem.replace("{n}", _params[i].name);
                pitem.replace("{p}", _params[i].label);
                snprintf(parLength, 5, "%d", _params[i].maxLength);
                pitem.replace("{l}", parLength);
                pitem.replace("{v}", _params[i].value);
                pitem.replace("{c}", _params[i].customHTML);
                page += pitem;
            }
        } 
    }
    page += FPSTR(HTTP_FORM_END);
    page += FPSTR(HTTP_SCAN_LINK);
    page += FPSTR(HTTP_END);
    _server->sendHeader("Content-Length", String(page.length()));
    _server->send(200, "text/html", page);
    log(F("Sent config page"));
}

void handleNotFound() {
    if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
        return;
    }
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += _server->uri();
    message += "\nMethod: ";
    message += _server->method() == HTTP_GET ? "GET" : "POST";
    message += "\nArguments: ";
    message += _server->args();
    message += "\n";
    for (uint8_t i = 0; i < _server->args(); i++) {
        message += " " + _server->argName(i) + ": " + _server->arg(i) + "\n";
    }
    _server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    _server->sendHeader("Pragma", "no-cache");
    _server->sendHeader("Expires", "-1");
    _server->sendHeader("Content-Length", String(message.length()));
    _server->send ( 404, "text/plain", message );
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
    log(F("WiFi save"));
    _ssid = _server->arg("s").c_str();
    _pass = _server->arg("p").c_str();
    //parameters
    for (int i = 0; i < _lastParam; i++) {
        //read parameter
        String value = _server->arg(_params[i].name);
        //store it in array
        value.toCharArray(_params[i].value, _params[i].maxLength);
        log(_params[i].name, value);
    }
    String page = FPSTR(HTTP_HEAD);
    page.replace("{v}", "Credentials Saved");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;
    page += FPSTR(HTTP_HEAD_END);
    page += FPSTR(HTTP_SAVED);
    page += FPSTR(HTTP_END);
    _server->sendHeader("Content-Length", String(page.length()));
    _server->send(200, "text/html", page);
    log(F("Sent wifi save page"));
    _connect = true; //signal ready to connect/reset
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
bool captivePortal() {
    if (!isIp(_server->hostHeader()) ) {
        log(F("Request redirected to captive portal"));
        _server->sendHeader("Location", String("http://") + toStringIp(_server->client().localIP()), true);
        _server->send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        _server->client().stop(); // Stop is needed because we sent no content length
        return true;
    }
    return false;
}

bool isIp(String str) {
    for (int i = 0; i < str.length(); i++) {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9')) {
        return false;
        }
    }
    return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
    String res = "";
    for (int i = 0; i < 3; i++) {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}

int getRSSIasQuality(int RSSI) {
    int quality = 0;
    if (RSSI <= -100) {
        quality = 0;
    } else if (RSSI >= -50) {
        quality = 100;
    } else {
        quality = 2 * (RSSI + 100);
    }
    return quality;
}

void saveConfig() {

}

/* Loads config from FS. Returns true if config exists false if not. */
bool loadConfig() {
    ConfigParam p = createCombo("moduleType", "Module type", PARAM_LENGTH, "required");
    p.options.push_back("Sensor");
    p.options.push_back("Light");
    p.options.push_back("Switch");
    addParam(p);
    addParam(createInput("moduleName", "Module name", "Test module", PARAM_LENGTH, "required"));
    addParam(createInput("moduleLocation", "Module location", "Test loction", PARAM_LENGTH, "required"));
    return false;
}

ConfigParam createCombo(const char* name, const char* label, uint8_t length, const char* customHTML) {
    ConfigParam p = {};
    p.type = Combo;
    p.name = name;
    p.label = label;
    p.maxLength = length;
    p.customHTML = customHTML;
    return p;
}

ConfigParam createInput(const char* name, const char* label, char* defVal, uint8_t length, const char* customHTML) {
    ConfigParam p = {};
    p.type = Text;
    p.name = name;
    p.label = label;
    p.value = defVal;
    p.customHTML = customHTML;
    p.maxLength = length;
    return p;
}

char* getStationName() {
    return new char[2];
}

void addParam (ConfigParam p) {
    if (_lastParam < MAX_PARAMS) {
        _params[_lastParam++] = p;
    } else {
        log(F("Max params reached"));
    }
}