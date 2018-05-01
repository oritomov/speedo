#ifndef common_h
#define common_h

typedef struct uint24_t {
  uint8_t low8;
  uint8_t mid8;
  uint8_t high8;
} uint24;

typedef struct flags_t {
  boolean lo : 1;
  boolean mi : 1;
  boolean hi : 1;
  boolean half : 1;
} flags;

uint16_t make16(uint24_t bigtimer);

uint16_t eeprom_calib_factor(void);
void eeprom_calib_factor(uint16_t);

uint16_t eeprom_pulses_in_100m(void);
void eeprom_pulses_in_100m(uint16_t);

uint16_t eeprom_trip(boolean);
void eeprom_trip(uint16_t, boolean);

uint24_t eeprom_distance(void);
void eeprom_distance(uint24_t, boolean, flags*);

#endif //common_h
