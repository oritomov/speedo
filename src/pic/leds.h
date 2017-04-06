#ifndef leds_h
#define leds_h

#include "display.h"

#define tmr2if					pir1.TMR2IF

// mask the SEVSEG_BITFIELD with the following to decide what bits get written to when writing out to display.
#define SEVSEG_DECIMAL_BIT		porta.6
#define SEVSEG_PORTB_MASK		0b11110111
#define SEVSEG_PORTA_MASK		0b00001111
//								      cabg
#define hundreds_driver			0b00000000 // y0
#define tens_driver				0b00000100 // y1
#define ones_driver				0b00000010 // y2
#define trip_hundreds_driver	0b00001000 // y4
#define trip_tens_driver		0b00000110 // y3
#define trip_ones_driver		0b00001100 // y5
#define trip_100m_driver		0b00001010 // y6
//								      bacg
#define distance_6_driver		0b00000101 // y1
#define distance_5_driver		0b00001001 // y2
#define distance_4_driver		0b00001011 // y6
#define distance_3_driver		0b00001101 // y3
#define distance_2_driver		0b00000111 // y5
#define distance_1_driver		0b00000011 // y4

void init_leds(void);
void refresh_sevenseg(void);
void interrupt_leds(void);

#endif leds_h
