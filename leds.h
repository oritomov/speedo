#include "display.h"

#define tmr2if          pir1.TMR2IF

// mask the SEVSEG_BITFIELD with the following to decide what bits get written to when writing out to display.
#define SEVSEG_PORTA_MASK                   0b11001111
#define SEVSEG_PORTB_MASK                   0b11110111
#define SEVSEG_DECIMAL_BIT                  porta.6
#define hundreds_driver                     0b00000110
#define tens_driver                         0b00000101
#define ones_driver                         0b00000111
#define distance_6_driver                   0b00001010
#define distance_5_driver                   0b00001001
#define distance_4_driver                   0b00001011
#define distance_3_driver                   0b10001000
#define distance_2_driver                   0b10001010
#define distance_1_driver                   0b10001001
#define trip_hundreds_driver                0b10000111
#define trip_tens_driver                    0b10000100
#define trip_ones_driver                    0b10000110
#define trip_100m_driver                    0b10000101

void init_led(void);
void refresh_sevenseg(void);
void interrupt_led(void);
