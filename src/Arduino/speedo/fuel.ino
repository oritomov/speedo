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

#ifdef fuel_h
// happens when we get a speedometer pulse.
void fuel_interrupt(void) {
  fuel.interrupt();
}

// init
void Fuel::init() {
  pinMode(LPG_INPUT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(LPG_INPUT), fuel_interrupt, CHANGE);
  flag_lpg_reed = digitalRead(LPG_INPUT);  // if LPG_INPUT is low - flag_lpg_reed is true.
}

void Fuel::interrupt(void) {
  flag_lpg_reed = digitalRead(LPG_INPUT);  // if LPG_INPUT is low - flag_lpg_reed is true.
}
#endif
