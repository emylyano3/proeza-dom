#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/* wifi config */
const char* ssid = "dd-wrt-low";
const char* password = "sabarasa";

/* mqqt borker config */
const char* mqtt_server = "192.168.0.105";

/* topics config */
const char* TOPIC_COMMAND = "light/room01/cmd";
const char* TOPIC_STATE = "light/room01/state";
const char* TOPIC_ONLINE = "ESP/online";

const unsigned int STATE_INVALID = -1;
const unsigned int STATE_ON = 1;
const unsigned int STATE_OFF = 0;

/* states map */
const int STATES_WRITE[2] = {HIGH, LOW};

WiFiClient espClient;
PubSubClient client(espClient);

char curState = 0;

void setup() {
  // Initialize the BUILTIN_LED pin as an output
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, unsigned char* payload, unsigned int length) {
  int state;
  if (STATE_INVALID == (state = translateMessage(topic, payload, length))) {
    Serial.print("Invalid state [");
    Serial.print(state);
    Serial.println("]");
    return;
  }
  if (state == curState) {
    Serial.println("No state change detected. Ignoring.");
  } else {
    changeState(state);
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

void changeState (unsigned int state) {
  switch (state) {
    case STATE_OFF:
      // Turn the LED OFF (Note that HIGH is the voltage level but actually the LED is OFF
      // This is because it is acive low on the ESP-01)
      digitalWrite(BUILTIN_LED, HIGH);
      client.publish(TOPIC_STATE, "0");
      curState = state;
      break;
    case STATE_ON:
      digitalWrite(BUILTIN_LED, LOW);
      client.publish(TOPIC_STATE, "1");
      curState = state;
      break;
    default:
      break;
  }
  Serial.print("State changed to: ");
  Serial.println(curState);
}

void connectBroker() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP_light-room01")) {
      Serial.println("connected");
      // once connected subscribe
      client.subscribe(TOPIC_COMMAND);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    connectBroker();
  }
  client.loop();
}
