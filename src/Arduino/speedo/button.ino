/*
Button subroutine.
version: 0.2

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
*/

#include "button.h"

#define BUTTON_HELD 500

// Interrupt is called once a millisecond, 
SIGNAL(TIMER0_COMPA_vect) {
  button.interrupt();
}

void Button::init() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  heldcount = 0;
  reset();

  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A); // Enable Output Compare Match A Interrupt
}

void Button::reset(void) {
  pressed = false;
  held = false;
}

void Button::interrupt(void) {
  // button polling routine (every 32.768 ms)

  // the flags will be cleared externally
  boolean button_input = digitalRead(BUTTON_PIN);

  if (!button_input) { // if button is pressed, BUTTON_INPUT is held low.
    heldcount++;
    
    // if the button has been on 30 times in a row (about 1 second), flag it as being held
    if (heldcount == BUTTON_HELD)
      held = true;
      
    // if the button has been on more than 30 times in a row, we've already flagged it as held
    // if the button has been on THIS long, the user is obviously on drugs
    else if (heldcount == 0xFFF)
      heldcount--; // so as not to overflow
  } else {
    // if the button hasn't been pressed, nothing to report.
    // if the button was released after being on once or twice, it was a glitch.
    // if the button was released after being on between 2 and 29 times in a row, it was pressed
    if ((heldcount > 1) && (heldcount < BUTTON_HELD))
      pressed = true;
      
    // if the button was released after being on more than that, we already flagged it as held.
    heldcount = 0;
  }
}

