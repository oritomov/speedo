#ifndef common_h
#define common_h

#include <system.h>

#pragma DATA _CONFIG, _LVP_OFF & _BOREN_ON & _MCLRE_OFF & _PWRTE_ON & _WDT_OFF & _INTOSC_OSC_NOCLKOUT

typedef struct _uns24 {
    unsigned char low8;
    unsigned char mid8;
    unsigned char high8;
} uns24;

#define gie             intcon.GIE

unsigned int make16(uns24 bigtimer);
unsigned int make16i(uns24 bigtimer);   // same thing, just used only in interrupts
unsigned char eeprom_read(unsigned char address);
void eeprom_write(unsigned char address, unsigned char data);

#endif common_h
