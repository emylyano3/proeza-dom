
char conf[EEPROM_SIZE] = "";

String readConf (char key[]) {
	if (sizeof(conf) < 1 || sizeof(key) < 1) {
		return "";
	}
	if ('|' != conf[0] || '|' != conf[sizeof(conf) - 1]) {
		return "";
	}
	int j = 0;
	int i = 1;
	for (; i < sizeof(conf); i++) {
		j = conf[i] == key[j] ? ++j : 0;
		if (j == sizeof(key)) {
      if (i + 1 < sizeof(conf) && ':' == conf[i + 1]) {
        // Salto del ultimo char correspondiente al key buscado al primero del value
  			i += 2;
  			break;
      } else {
        j = 0;
      }
		}
	}
	if (i < sizeof(conf)) {
		j = i;
		while ('|' != conf[j] && i < sizeof(conf)) {
			++j;
		}
		char value[j-i];
		j = 0;
		while ('|' != conf[i] && i < sizeof(conf) && j < sizeof(value)) {
			value[j++] = conf[i++];
		}
		Serial.print("Value read: ");
		Serial.println(value);
		return String(value);
	} else {
		return "";
	}
}

void saveConf(char conf[]) {
	Serial.print("Saving configuration: ");
	Serial.println(conf);
	EEPROM.begin(EEPROM_SIZE);
	// Plancho la conf anterior
	for (size_t i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  for (size_t i = 0; i < sizeof(conf); i++) {
    EEPROM.write(i, conf[i]);
  }
  EEPROM.commit();
  EEPROM.end();
}

void loadConf () {
  EEPROM.begin(EEPROM_SIZE);
	Serial.println("Loading config from EEPROM");
	for (size_t i = 0; i < EEPROM_SIZE; ++i) {
		conf[i] = EEPROM.read(i);
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
  EEPROM.begin(EEPROM_SIZE);
  for (size_t i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.end();
}