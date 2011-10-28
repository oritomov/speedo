#ifndef button_h
#define button_h

#include "hodo.h"

#define BUTTON_INPUT	porta.5

#define t0if			intcon.T0IF

void init_button(void);
void reset_button(void);
void interrupt_button(void);

extern bit flag_buttonpressed;
extern bit flag_buttonheld;
extern unsigned char button_heldcount;

#endif button_h
