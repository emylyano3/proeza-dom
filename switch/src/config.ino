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
  Serial.println(strlen(pass) > 0 ? "********" : "<no pass>");
  Serial.println(moduleName);
}

/** Store credentials to EEPROM */
void saveConfig() {
  Serial.println("Saving config");
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), pass);
  EEPROM.put(0 + sizeof(ssid) + sizeof(pass), moduleName);
  Serial.printf("SSID: [%s] Module Name: [%s] Pass: [********]\n", ssid, moduleName);
  char result[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(ssid) + sizeof(pass) + sizeof(moduleName), result);
  EEPROM.commit();
  EEPROM.end();
}
