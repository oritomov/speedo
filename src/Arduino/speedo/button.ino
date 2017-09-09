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

// Interrupt is called once a millisecond, 
SIGNAL(TIMER0_COMPA_vect) {
  button.interrupt();
}

void Button::init() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  held_count = 0;
  reset();

  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A); // Enable Output Compare Match A Interrupt
}

void Button::reset(void) {
  rlsd_count = 0;
  status = BUTTON_STATUS_NONE;
  Serial.println("button init");
}

// button polling routine (every 32.768 ms)
void Button::interrupt(void) {

  // the flags will be cleared externally
  boolean button_input = digitalRead(BUTTON_PIN);

  if (!button_input) { // if button is pressed, BUTTON_INPUT is held low.
    held_count++;
    
    // if the button has been on 30 times in a row (about 1 second), flag it as being held
    if (held_count == BUTTON_HELD) {
      status = BUTTON_STATUS_HELD;
      Serial.println("button held");
      
    // if the button has been on more than 30 times in a row, we've already flagged it as held
    // if the button has been on THIS long, the user is obviously on drugs
    } else if (held_count == 0xFFF) {
      held_count--; // so as not to overflow
    }

    rlsd_count = 0;
  } else {
    // if the button hasn't been pressed, nothing to report.
    // if the button was released after being on once or twice, it was a glitch.
    // if the button was released after being on between 2 and 29 times in a row, it was pressed
    if ((held_count > 1) && (held_count < BUTTON_HELD)) {
      status = BUTTON_STATUS_PRESSED;
      Serial.println("button pressed");
    }
    // if the button was released after being on more than that, we already flagged it as held.
    held_count = 0;

    rlsd_count++;
    if (rlsd_count == BUTTON_DELAY) {
      status = BUTTON_STATUS_TIMEOUT;
      Serial.println("button time out");
    } else if (rlsd_count > BUTTON_DELAY) {
      rlsd_count--; // so as not to overflow
    }
  }
}

