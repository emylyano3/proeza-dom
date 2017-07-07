/** Load config from EEPROM */
void loadConfig () {
  Serial.println("Loading configuration");
  EEPROM.begin(512);
  EEPROM.get(0, ssid);
  EEPROM.get(0 + sizeof(ssid), pass);
  EEPROM.get(0 + sizeof(ssid) + sizeof(pass), moduleName);
  char result[2 + 1];
  EEPROM.get( 0 + sizeof(ssid) + sizeof(pass) + sizeof(moduleName), result);
  EEPROM.end();
  if (String(result) != String("OK")) {
    ssid[0] = 0;
    pass[0] = 0;
    moduleName[0] = 0;
  }
  Serial.println("Recovered Data:");
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(moduleName);
  // Serial.println(strlen(pass) > 0 ? "********" : "<no pass>");
  setState(setupDone() ? STATE_RUN : STATE_SETUP);
}

boolean setupDone () {
  if (strlen(ssid) > 0 && String(ssid) != String("none")) {
    Serial.println("SSID is set");
    if (strlen(moduleName) > 0 && String(moduleName) != String("ESP_Switch")) {
      Serial.println("Module name is set");
      return true;
    }
    Serial.println("Module name is not set");
  }
  Serial.println("SSID is not set");
  return false;
}

/** Store credentials to EEPROM */
void saveConfig() {
  Serial.println("Saving config");
  // EEPROM.begin(512);
  // EEPROM.put(0, ssid);
  // EEPROM.put(0 + sizeof(ssid), pass);
  // EEPROM.put(0 + sizeof(ssid) + sizeof(pass), moduleName);
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Module Name: ");
  Serial.println(moduleName);
  Serial.print("Password: ");
  Serial.println(pass);
  // char result[2 + 1] = "OK";
  // EEPROM.put(0 + sizeof(ssid) + sizeof(pass) + sizeof(moduleName), result);
  // EEPROM.commit();
  // EEPROM.end();
  setState(STATE_RUN);
}
