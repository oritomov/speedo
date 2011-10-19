#ifndef speed_h
#define speed_h

#include "hodo.h"

#define SPEEDO_INPUT                        portb.3

// how many overflows of tmr1 before a pulse is considered "stale" (invalid)
#define PULSE_LIFETIME          7
// 62 bigticks = 1.98ms
#define SPEEDO_BOUNCE_THRESHOLD 62

#define tmr1if          pir1.TMR1IF
#define ccp1if          pir1.CCP1IF

void init_speed(void);
void calculate_speed(bit forceupdate);
void interrupt_speed(void);
unsigned char tmr1_upper(void);
unsigned int pulsecount(void);
void pulsecount(unsigned int num);
unsigned int speed(void);

#endif speed_h
