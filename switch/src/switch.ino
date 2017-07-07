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

long nextBrokerConnAtte = 0;
long nextStatsPrint = 0;

bool stationConnected = false;
bool mqttServerConfigured = false;

int curSwitchState = 0;

void moduleRun () {
  if (!stationConnected) {
    stationConnected = connectStation();
  }
  if (!mqttServerConfigured) {
    mqttServerConfigured = configureMqttServer();
  }
  if (!mqttClient.connected()) {
    connectBroker();
  }
  mqttClient.loop();
}

bool configureMqttServer () {
  mqttClient.setServer(mqttServerIP, mqttServerPort);
  mqttClient.setCallback(callback);
}

bool connectStation () {
  WiFi.mode(WIFI_STA);
  Serial.printf("Connecting station to ssid: %s pass: %s", ssid, pass);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

void callback(char* topic, unsigned char* payload, unsigned int length) {
  int state;
  if (STATE_INVALID == (state = translateMessage(topic, payload, length))) {
    Serial.printf("Invalid state [%s]\n", state);
    return;
  }
  if (state == curSwitchState) {
    Serial.println("No state change detected. Ignoring.");
  } else {
    changeSwitchState(state);
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
  Serial.print("Payload: " );
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
      mqttClient.publish(TOPIC_STATE, "0");
      curSwitchState = state;
      break;
    case STATE_ON:
      digitalWrite(BUILTIN_LED, LOW);
      mqttClient.publish(TOPIC_STATE, "1");
      curSwitchState = state;
      break;
    default:
      break;
  }
  Serial.printf("State changed to: %d\n", curSwitchState);
}

void printStats () {
  if (nextStatsPrint <= millis()) {
    nextStatsPrint = millis() + LOOP_THRESHOLD;
    WiFi.printDiag(Serial);
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
  }
}

void connectBroker() {
  if (nextBrokerConnAtte <= millis()) {
    nextBrokerConnAtte = millis() + LOOP_THRESHOLD;
  // while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(moduleName)) {
      Serial.println("connected");
      // once connected subscribe
      mqttClient.subscribe(TOPIC_COMMAND);
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqttClient.state());
      // Serial.println(" try again in 5 seconds");
    }
  }
  // }
}
