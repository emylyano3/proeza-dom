
const uint16_t CONFIG_SIZE = 256;

char* CONFIG = (char*) malloc(CONFIG_SIZE);

char* readConf (const char* key, char* buff) {
	Serial.printf("Reading key '%s' from config.\n", key);
	if (!CONFIG || !key || !strlen(key)) {
		Serial.println("No config found or invalid key.");
		return NULL; 
	}
	if ('|' != CONFIG[0] || '|' != CONFIG[strlen(CONFIG)-1]) {
		Serial.println(CONFIG);
		Serial.println("Invalid config found. Cleaning config.");
		cleanConfig();
		return NULL;
	}
	int j = 0;
	int i = 1;
	for (; i < CONFIG_SIZE; i++) {
		j = CONFIG[i] == key[j] ? ++j : 0;
		if (j == strlen(key)) {
      if (i + 1 < CONFIG_SIZE && ':' == CONFIG[i + 1]) {
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
		while (j < CONFIG_SIZE && '|' != CONFIG[j]) {
			++j;
		}
		uint16_t valueSize = j-i;
		// Reset de j
		j = 0; 
		while (i < CONFIG_SIZE && j < valueSize && '|' != CONFIG[i]) {
			buff[j++] = CONFIG[i++];
		}
		buff[valueSize] = '\0';
		Serial.printf("Value for key '%s': %s.\n", key, buff);
		return buff;
	}
	return NULL;
}

void saveConf(const char* cf) {
	Serial.printf("Saving configuration to EEPROM: %s.\n", cf);
	cleanConfig();
	EEPROM.begin(CONFIG_SIZE);
	uint16_t confSize = strlen(cf);
	Serial.printf("Config size: %d.\n", confSize);
  for (size_t i = 0; i < confSize; i++) {
    EEPROM.write(i, cf[i]);
	}
  EEPROM.commit();
  EEPROM.end();
}

void loadConf () {
	Serial.println("Loading config from EEPROM");
  EEPROM.begin(CONFIG_SIZE);
	for (size_t i = 0; i < CONFIG_SIZE; ++i) {
		CONFIG[i] = EEPROM.read(i);
	}
	EEPROM.end();
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
		CONFIG[i] = 0;
  }
  EEPROM.commit();
  EEPROM.end();
}