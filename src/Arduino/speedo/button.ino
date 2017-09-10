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

// Interrupt is called once in a millisecond 
SIGNAL(TIMER0_COMPA_vect) {
  button.interrupt();
}

void Button::init() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  held_count = 0;
  reset();

  cli();//stop interrupts

  // Timer0 is an 8-bit that counts from 0 to 255 and generates an interrupt whenever it overflows. 
  // It uses a clock divisor of 64 by default to give us an interrupt rate of 976.5625 Hz (close enough to a 1KHz for our purposes).  
  // We won't mess with the freqency of Timer0, because that would break millis()!   

  // set timer0 interrupt at 1kHz
  //TCCR0A = 0;// set entire TCCR0A register to 0
  //TCCR0B = 0;// same for TCCR0B
  //TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 1khz increments
  //OCR0A = 249;// = (16*10^6) / (1000*64) - 1 (must be <256)
  // turn on CTC mode
  //TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  //TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

  sei();//allow interrupts
}

void Button::reset(void) {
  rlsd_count = 0;
  status = BUTTON_STATUS_NONE;
  Serial.println("button init");
}

// button polling routine (every 1 ms)
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

