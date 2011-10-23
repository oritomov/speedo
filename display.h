#ifndef display_h
#define display_h

#include "speed.h"

// This macro translates the order of the 7-segment bits to whatever the hardware pins are actually assigned to.
// input is from H to A, H being the decimal and A being the top LED, etc (see 7 segment documentation)
// This lets us change the wiring without altering all the bitwise code.
#define SEVSEG_BITFIELD(H,G,F,E,D,C,B,A)    0b ## C ## E ## D ## F ## H ## A ## B ## G
#define SEVSEG_DECIMAL_MASK                 SEVSEG_BITFIELD(1,0,0,0,0,0,0,0)

#define _A SEVSEG_BITFIELD(0,1,1,1,0,1,1,1)
#define _C SEVSEG_BITFIELD(0,0,1,1,1,0,0,1)
#define _d SEVSEG_BITFIELD(0,1,0,1,1,1,1,0)
#define _E SEVSEG_BITFIELD(0,1,1,1,1,0,0,1)
#define _G SEVSEG_BITFIELD(0,0,1,1,1,1,0,1)
#define _L SEVSEG_BITFIELD(0,0,1,1,1,0,0,0)
#define _n SEVSEG_BITFIELD(0,1,0,1,0,1,0,0)
#define _o SEVSEG_BITFIELD(0,1,0,1,1,1,0,0)
#define _P SEVSEG_BITFIELD(0,1,1,1,0,0,1,1)
#define _r SEVSEG_BITFIELD(0,1,0,1,0,0,0,0)
#define _S SEVSEG_BITFIELD(0,1,1,0,1,1,0,1)
#define _t SEVSEG_BITFIELD(0,1,1,1,1,0,0,0)
#define _u SEVSEG_BITFIELD(0,0,0,1,1,1,0,0)

void check_display(void);
unsigned char sevenseg_lookup(unsigned char digit);
void display_speed(unsigned int speed);
void display_trip(unsigned int trip);
void display_distance(uns24 num);
unsigned char sevenseg_bits_i(unsigned char current_digit);
unsigned char sevenseg_bits(unsigned char current_digit);
void sevenseg_bits(unsigned char current_digit, unsigned char bits);

#endif display_h
