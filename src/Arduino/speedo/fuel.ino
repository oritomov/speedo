/*
Fuel subroutine.
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

#include "fuel.h"

// happens when we get a speedometer pulse.
void interrupt_lpg(void) {
  fuel.interrupt();
}

// init
void Fuel::init(int lpg_pin) {
  Fuel::lpg_pin = lpg_pin;
  flag_lpg_reed = false;
  flag_lpg_mode = false;

  pinMode(lpg_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(lpg_pin), interrupt_lpg, CHANGE);
}

void Fuel::interrupt(void) {
  flag_lpg_reed = !digitalRead(lpg_pin);  // if LPG is on, LPG_INPUT is held low.
}

