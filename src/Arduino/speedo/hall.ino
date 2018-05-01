/*
Hall read subroutine.
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

// 1 << 5 = 32
#define DIVIDER 5

// how many overflows of tmr1 before a pulse is considered "stale" (invalid)
#define PULSE_LIFETIME      7

// 62 bigticks = 2us >> 5
#define SPEEDO_BOUNCE_THRESHOLD  62

// Interrupt is called once in a millisecond 
ISR(TIMER1_COMPA_vect) {
  hall.timer_interrupt();
}

// happens when we get a hall pulse.
void interrupt_hall(void) {
  hall.hall_interrupt();
}

void Hall::init() {
  pulsecount = 0;
  
  lastpulse_stale0 = true; // both pulses are invalid on reset; we need to get TWO pulses in a reasonable time
  lastpulse_stale1 = true; // before doing any calculating.

  tmr1_upper = 0;

  // initialize timer1 
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 31249;                        // compare match register 16MHz/256/2Hz
                                
  TCCR1B |= (1 << WGM12);               // CTC mode
  TCCR1B |= (1 << CS12);                // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);              // enable timer compare interrupt
  
  pinMode(HALL_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), interrupt_hall, FALLING);
}

boolean Hall::stale(void) {
  return lastpulse_stale1 || lastpulse_stale0;
}

void Hall::get_lastpulses(uint16_t * temp_lastpulses) {
  // stop interrupt for a sec... we don't want the clocks messed with
  detachInterrupt(digitalPinToInterrupt(HALL_PIN));

  temp_lastpulses[1] = make16(lastpulses[1]);
  temp_lastpulses[0] = make16(lastpulses[0]);
    
  // start the interrupt again
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), interrupt_hall, FALLING);
}

// handles timer interrupt every 0.5 sec
void Hall::timer_interrupt(void) {
  // tmr1 overflow, increment tmr1 "most significant bits"
  tmr1_upper++;

  // expire stale pulses
  if (tmr1_upper - lastpulses[1].high8 > PULSE_LIFETIME) {
    lastpulse_stale1 = true;
  }
  if (tmr1_upper - lastpulses[0].high8 > PULSE_LIFETIME / 2) {
    lastpulse_stale0 = true;
  }
}

// handles hall interrupt, happens when we get a speedometer pulse.
void Hall::hall_interrupt(void) {
  uint24_t now;
  uint16_t time = micros() >> DIVIDER;
  now.high8 = tmr1_upper;
  now.mid8 = highByte(time);
  now.low8 = lowByte(time);

  // if it's too close to the last pulse, ignore it, it's just a bouncy switch
  if ((make16(now) - make16(lastpulses[0])) > SPEEDO_BOUNCE_THRESHOLD) { // 2ms
#ifdef hodo_h
    hodo.incr_distance();
#endif // hodo_h

    pulsecount++;
    lastpulses[1] = lastpulses[0];
    lastpulse_stale1 = lastpulse_stale0;
    lastpulses[0] = now;
    lastpulse_stale0 = false;
  }
}