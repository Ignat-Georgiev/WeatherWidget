#ifndef ESPClock_Settings_h
#define ESPClock_Settings_h

#include <IPAddress.h>
#include <EEPROM.h>

struct SettingsClass {
  static const uint32_t FLASH_MAGIC_R1 = 0x5aa5e001;  // Before adding multiple presets
  static const uint32_t FLASH_MAGIC_R2 = 0x5aa5e002;  // Before adding OpenWeatherMaps API key
  static const uint32_t FLASH_MAGIC = 0x5aa5e003;
  
  uint32_t magic;
  char ssid[32];  // ESP seems to have a 31-char limit
  char password[64];
  char hostname[32];
  int utcOffset;  
  char country[32];
  char city[32];
  char apikey[17];

  bool alarm1;
  bool alarm2;
  bool alarm3;

  char alarm1time[32];
  char alarm2time[32];
  char alarm3time[32];

  char days1[32];
  char days2[32];
  char days3[32];

  void begin();
  void save();
};

extern SettingsClass Settings;

#endif  // ESPClock_Settings_h

