#include <Arduino.h>
#include <EEPROM.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "Settings.h"

SettingsClass Settings;

static const size_t EEPROM_SIZE = 512;

void SettingsClass::begin() {
#if defined(ESP8266)
  EEPROM.begin(EEPROM_SIZE);
#else
  EEPROM.begin();
#endif
  EEPROM.get(0, *this);

  // Populate defaults if flash magic is unexpected
  if (magic != FLASH_MAGIC) {
    if (magic != FLASH_MAGIC_R2) {
      if (magic != FLASH_MAGIC_R1) {
        bzero(this, sizeof(*this));
      }
    }
    bzero(apikey, sizeof(apikey));
	bzero(country, sizeof(country));
	bzero(city, sizeof(city));

    magic = FLASH_MAGIC;
    save();  // Is this really necessary?
  }
}

void SettingsClass::save() {
  EEPROM.put(0, *this);
#if defined(ESP8266)
  EEPROM.commit();
#endif
}

