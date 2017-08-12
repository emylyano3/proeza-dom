/* MQTT Client */
WiFiClient espClient;
PubSubClient mqttClient(espClient);

/* Topics config */
const char* TOPIC_COMMAND = "light/room01/cmd";
const char* TOPIC_STATE = "light/room01/state";

const int LOOP_THRESHOLD = 3000;

/* Possible switch states */
const int STATE_OFF     = 0;
const int STATE_ON      = 1;
const int STATE_INVALID = 9;

int currSwitchState = STATE_OFF;

long nextBrokerConnAtte = 0;
long nextStatsPrint = 0;

bool stationConnected = false;
bool mqttBrokerConfigured = false;

void moduleRun () {
  if (!stationConnected) {
    if (wrongConfiguration(connectStation())) {
      WiFi.mode(WIFI_OFF);
      delay(100);
      setState(STATE_SETUP);
      return;
    } else {
      stationConnected = true;
    }
  }
  if (!mqttBrokerConfigured) {
    mqttBrokerConfigured = configureMqttBroker();
  }
  if (!mqttClient.connected()) {
    connectBroker();
  }
  mqttClient.loop();
  printStats();
}

bool wrongConfiguration (int status) {
    return status == WL_CONNECT_FAILED; // || status == WL_NO_SSID_AVAIL;
}

bool configureMqttBroker () {
  Serial.printf("Configuring MQTT broker. Server: %s. Port: %s\n", mqttBrokerIP, mqttBrokerPort);
  mqttClient.setServer(mqttBrokerIP, mqttBrokerPort);
  mqttClient.setCallback(callback);
  return true;
}

int connectStation() {
  Serial.println("Connecting station");
  WiFi.mode(WIFI_STA);
  WiFi.hostname(moduleName);
  WiFi.begin(ssid, pass);
  // WiFi.hostname(value);
  // WiFi.begin("dd-wrt-low", "sabarasa");
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
  int state;
  if (STATE_INVALID == (state = translateMessage(topic, payload, length))) {
    Serial.printf("Invalid state [%s]\n", state);
    return;
  }
  if (state == currSwitchState) {
    Serial.println("No state change detected. Ignoring.");
  } else {
    changeSwitchState(state);
  }
  publishSwitchState();
}

void publishSwitchState () {
    switch (currSwitchState) {
      case STATE_OFF:
        mqttClient.publish(TOPIC_STATE, "0");
        return;
      case STATE_ON:
        mqttClient.publish(TOPIC_STATE, "1");
        return;
      default: return;
    }
}

int translateMessage (char* topic, unsigned char* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  Serial.print("Length: " );
  Serial.println(length);
  if (length != 1) {
    Serial.println("Invalid payload length. Ignoring.");
    return STATE_INVALID;
  }
  Serial.print("Payload: ");
  Serial.println(payload[0]);
  if (!isDigit(payload[0])) {
      Serial.println("Invalid payload. Ignoring.");
      return STATE_INVALID;
  }
  return payload[0] == '1' ? STATE_ON : payload[0] == '0' ? STATE_OFF : STATE_INVALID;
}

void changeSwitchState (unsigned int state) {
  switch (state) {
    case STATE_OFF:
      // Turn the LED OFF (Note that HIGH is the voltage level but actually the LED is OFF
      // This is because it is acive low on the ESP-01)
      digitalWrite(BUILTIN_LED, HIGH);
      currSwitchState = state;
      break;
    case STATE_ON:
      digitalWrite(BUILTIN_LED, LOW);
      currSwitchState = state;
      break;
    default:
      break;
  }
  Serial.printf("State changed to: %d\n", currSwitchState);
}

void printStats () {
  if (nextStatsPrint <= millis()) {
    nextStatsPrint = millis() + (LOOP_THRESHOLD << 6);
    Serial.println("...............................................................");
    WiFi.printDiag(Serial);
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    Serial.println("...............................................................");
  }
}

void connectBroker() {
  if (nextBrokerConnAtte <= millis()) {
    nextBrokerConnAtte = millis() + LOOP_THRESHOLD;
    Serial.printf("Connecting MQTT broker as %s...", moduleName);
    yield();
    if (mqttClient.connect(moduleName)) {
      Serial.println("connected");
      mqttClient.subscribe(TOPIC_COMMAND);
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqttClient.state());
    }
  }
}
