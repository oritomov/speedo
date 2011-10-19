/*
Display subroutine.
version: 0.1

This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.



This program should compile under SourceBoost.  Feel free to modify it
to compile with any program you wish, for any platform you wish.
*/

#include "button.h"

// ram 
bit flag_buttonpressed;             // used to pass the message that a button press was registered
bit flag_buttonheld;                // ... or a button was held.  Clear manually before and after use.
unsigned char button_heldcount;     // how many tmr0 ticks the button has been held

void init_button(void) {
    button_heldcount = 0;
}

void reset_button(void) {
    flag_buttonpressed = 0;
    flag_buttonheld = 0;
}

unsigned char button_heldcount(void)
{
    return button_heldcount;
}

bit is_buttonpressed(void)
{
	return flag_buttonpressed;
}

bit is_buttonheld(void)
{
	return flag_buttonheld;
}

void interrupt_button(void)
{
    if (t0if)   // button polling routine (every 32.768 ms)
    {
        // the flags will be cleared externally
        if (!BUTTON_INPUT)  // if button is pressed, BUTTON_INPUT is held low.
        {
            button_heldcount++;
            // if the button has been on 30 times in a row (about 1 second), flag it as being held
            if (button_heldcount == 30)
                flag_buttonheld = 1;
            // if the button has been on more than 30 times in a row, we've already flagged it as held
            // if the button has been on THIS long, the user is obviously on drugs
            else if (button_heldcount == 0xFF)
                button_heldcount--; // so as not to overflow
        }
        else
        {
            // if the button hasn't been pressed, nothing to report.
            // if the button was released after being on once or twice, it was a glitch.
            // if the button was released after being on between 2 and 29 times in a row, it was pressed
            if ((button_heldcount > 1) && (button_heldcount < 30))
                flag_buttonpressed = 1;
            // if the button was released after being on more than that, we already flagged it as held.
            button_heldcount = 0;
        }

#ifdef _LPG
        interrupt_lpg();
#endif //_LPG

        t0if = 0;
    }
}
