/*
Hodo subroutine.
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

#include "hodo.h"
#include "eeprom.h"

// init
void Hodo::init() {
  flag_distance.lo = false; 
  flag_distance.mi = false;
  flag_distance.hi = false;
  flag_distance.half = false;
  flag_trip = false;
  flag_trip_half = false;

  pulses_in_100m = eeprom_pulses_in_100m();
  distance = eeprom_distance();

  distance_pulses = 0;

  read_trip(fuel.flag_lpg_mode);
}

// reset the trip
void Hodo::reset(void) {
  //Serial.println("Hodo::reset");
  trip = 0;
  trip_pulses = 0;
  eeprom_trip(trip, fuel.flag_lpg_mode);
  flag_trip = false;
  flag_trip_half = false;
}

// reads the trip fom EEPROM
void Hodo::read_trip(boolean flag_lpg_mode) {
  trip = eeprom_trip(flag_lpg_mode);
  trip_pulses = 0;
}

// increments distance on every ten 100m
void Hodo::incr_distance(void) {
  trip_pulses++;
  if (trip_pulses >= pulses_in_100m) {
    trip_pulses = 0;
    trip++;
    if (trip > 9999) {
      trip = 0;
    }
    flag_trip = true;  // write to eprom

    distance_pulses++;
    if (distance_pulses >= 10) {
      distance_pulses = 0;
      distance.low8++;
      if (distance.low8 >= 100) {
        distance.low8 = 0;
        distance.mid8++;
        if (distance.mid8 >= 100) {
          distance.mid8 = 0;
          distance.high8++;
          if (distance.high8 >= 100) {
            distance.high8 = 0;
          }
          flag_distance.hi = true;
        }
        flag_distance.mi = true;
      }
      flag_distance.lo = true;
    }
  } 
}

// Write a distance to the EEPROM.
void Hodo::write_distance(void) {
  // stores distance when it is xxxx.5
  if ((trip_pulses >= (pulses_in_100m / 2)) && !flag_trip_half) {
    eeprom_trip(trip + 1, fuel.flag_lpg_mode);
    // stored. don't store anymore
    flag_trip_half = true;
  }
  if (flag_trip) {
    // ready to store on next half
    flag_trip = false;
    flag_trip_half = false;
  }

  // stores distance when it is xxxx.5
  eeprom_distance(distance, distance_pulses == 5, &flag_distance);
}

