#include "hodo.h"

#define BUTTON_INPUT                        porta.5

#define t0if            intcon.T0IF

void init_button(void);
void reset_button(void);
unsigned char button_heldcount(void);
bit is_buttonpressed(void);
bit is_buttonheld(void);
void interrupt_button(void);
