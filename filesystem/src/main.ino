#include "FS.h"

long nextCheck = 0;
uint8_t state = 0;

void setup() {
  Serial.begin(115200);
  delay(15000);
  initFS();
}

void initFS () {
  String realSize = String(ESP.getFlashChipRealSize());
  String ideSize = String(ESP.getFlashChipSize());
  if (realSize.equals(ideSize)) {
    Serial.print("flash correctly configured. SPIFFS opened: ");
    Serial.println(SPIFFS.begin());;
  } else {
    Serial.println("flash incorrectly configured, SPIFFS cannot start, IDE size: " + ideSize + ", real size: " + realSize);
    while (1);
  }
}

void loop() {
  if (nextCheck < millis()) {
    nextCheck = millis() + 5000;
    switch (state) {
      case 0:
        checkFile();
      return;
      case 1:
        createFile();
      return;
      default:
        Serial.printf("Fatal error: Invalid state: %d\n", state);
        while(1);
    }
  }
}

void checkFile () {
  if (exists(String("/ssid.txt"))) {
    Serial.println("/ssid.txt exist");
    readFile();
  } else {
    Serial.println("/ssid.txt does not exist");
    state = 1;
  }
}

void readFile () {
  File f = SPIFFS.open("/ssid.txt", "r");
  if (!f) {
    Serial.println("Could not open file /ssid.txt for reading");
  } else {
    char bytes[32] = "";
    f.readBytes(bytes, f.available());
    Serial.println(bytes);
  }
  f.close();
}

void createFile () {
  File f = SPIFFS.open("/ssid.txt", "w");
  if (!f) {
    Serial.println("Could not open file /ssid.txt for writing");
  } else {
    f.println("dd-wrt-low");
    state = 0;
  }
  f.close();
}

bool exists (String file) {
  return SPIFFS.exists(file);
}
