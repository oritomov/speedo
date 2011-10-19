#include <system.h>

#ifndef _uns24
#define _uns24
typedef struct _uns24 {
    unsigned char low8;
    unsigned char mid8;
    unsigned char high8;
} uns24;
#endif

#define gie             intcon.GIE

unsigned int make16(uns24 bigtimer);
unsigned int make16i(uns24 bigtimer);   // same thing, just used only in interrupts
unsigned char eeprom_read(unsigned char address);
void eeprom_write(unsigned char address, unsigned char data);
