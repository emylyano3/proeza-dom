const char* CONF_LOCATION = "conf.txt";

const uint16_t CONFIG_SIZE = 256;

void moduleConfig() {
	Serial.println("Module configuration.");
  if (existConfig()) {
    loadConfig();
  } else {
    Serial.println("It does not exist persisted configuration. Going into setup mode.");
    setState(STATE_SETUP);
  }
}

void loadConfig () {
  Serial.println("Loading config from EEPROM");
  char* config = (char*) malloc(CONFIG_SIZE);
  EEPROM.begin(CONFIG_SIZE);
	for (size_t i = 0; i < CONFIG_SIZE; ++i) {
		config[i] = EEPROM.read(i);
	}
  EEPROM.end();
  readConf(config, "ssid", ssid);
  readConf(config, "pass", pass);
  readConf(config, "name", moduleName);
	readConf(config, "doma", moduleDomain);
	Serial.printf("ssid: %s pass: %s name: %s domain: %s\n", ssid, pass, moduleName, moduleDomain);
  free(config);
  setState(configOK() ? STATE_RUN : STATE_SETUP);
}

/** Del config string lee la key y la asigna al buffer */
void readConf (const char* config, const char* key, char* buff) {
	Serial.printf("Reading key '%s' from config.\n", key);
	if (!config || !key || !strlen(key)) {
		Serial.println("No config found or invalid key.");
		return; 
	}
	if ('|' != config[0] || '|' != config[strlen(config)-1]) {
		Serial.println(config);
		Serial.println("Invalid config found. Cleaning config.");
		cleanConfig();
		return;
	}
	int j = 0;
	int i = 1;
	for (; i < CONFIG_SIZE; i++) {
		j = config[i] == key[j] ? ++j : 0;
		if (j == strlen(key)) {
      if (i + 1 < CONFIG_SIZE && ':' == config[i + 1]) {
        // Salto del ultimo char correspondiente al key buscado al primero del value
  			i += 2;
  			break;
      } else {
        j = 0;
      }
		}
	}
	if (i < CONFIG_SIZE) {
		Serial.printf("Key '%s' found in config.\n", key);
		j = i;
		while (j < CONFIG_SIZE && '|' != config[j]) {
			++j;
		}
		uint16_t valueSize = j-i;
		// Reset de j
		j = 0; 
		while (i < CONFIG_SIZE && j < valueSize && '|' != config[i]) {
			buff[j++] = config[i++];
		}
		buff[valueSize] = '\0';
		Serial.printf("Value for key '%s': %s.\n", key, buff);
	}
}

void persistConfig () {
	// Serial.printf("Saving configuration to EEPROM: %s.\n", cf);
	// cleanConfig();
	// EEPROM.begin(CONFIG_SIZE);
	// uint16_t confSize = strlen(cf);
	// Serial.printf("Config size: %d.\n", confSize);
  // for (size_t i = 0; i < confSize; i++) {
  //   EEPROM.write(i, cf[i]);
	// }
  // EEPROM.commit();
	// EEPROM.end();
	setState(STATE_RUN);
}

bool existConfig() {
  EEPROM.begin(1);
  bool e = ((char) EEPROM.read(0)) == '|'; // caracter de inicio de config
  EEPROM.end();
  return e;
}

void cleanConfig() {
  EEPROM.begin(CONFIG_SIZE);
  for (size_t i = 0; i < CONFIG_SIZE; i++) {
		EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.end();
}

bool configOK () {
  //TODO Verificar todos los puntos de configuracion obligatorios
  if (ssid && strlen(ssid) > 0) {
    Serial.println("SSID is set");
    if (moduleName && strlen(moduleName) > 0) {
      Serial.println("Module name set");
      return true;
    }
    Serial.println("Module name not set");
  }
  Serial.println("SSID not set");
  return false;
}