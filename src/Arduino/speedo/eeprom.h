#ifndef eeprom_h
#define eeprom_h

unsigned int eeprom_calib_factor(void);
void eeprom_calib_factor(unsigned int);

unsigned int eeprom_pulses_in_100m(void);
void eeprom_pulses_in_100m(unsigned int);

unsigned int eeprom_trip(boolean);
void eeprom_trip(unsigned int, boolean);

_uns24 eeprom_distance(void);
void eeprom_distance(_uns24, boolean, flags*);

#endif //eeprom_h
