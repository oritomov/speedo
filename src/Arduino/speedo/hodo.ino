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

#include <EEPROM.h>
#include "hodo.h"

// eeprom
#define EEPROM_PULSES_IN_100M_LO  2
#define EEPROM_PULSES_IN_100M_HI  3

#define EEPROM_DISTANCE_LO      4
#define EEPROM_DISTANCE_MID     5
#define EEPROM_DISTANCE_HI      6

#define EEPROM_TRIP_LO        7
#define EEPROM_TRIP_HI        8
#define EEPROM_LPG_LO       9
#define EEPROM_LPG_HI       10

// init
void Hodo::init() {
  flag_distance_lo = false; 
  flag_distance_mi = false;
  flag_distance_hi = false;
  flag_distance_half = false;
  flag_trip = false;
  flag_trip_half = false;

  // pulses_in_100m
  {
    byte hi, lo;
    hi = EEPROM.read(EEPROM_PULSES_IN_100M_HI);
    lo = EEPROM.read(EEPROM_PULSES_IN_100M_LO);
    pulses_in_100m = word(hi, lo);
    Serial.print("pulses_in_100m = ");
    Serial.println(pulses_in_100m);
//    if (pulses_in_100m == 0xffff)
//      pulses_in_100m = DEFAULT_PULSES_IN_100M;
//    byte hi = highByte(pulses_in_100m);
//    byte lo = lowByte(pulses_in_100m);
//    EEPROM.write(EEPROM_PULSES_IN_100M_HI, hi);
//    EEPROM.write(EEPROM_PULSES_IN_100M_LO, lo);
  }

  // distance
  {
    distance.low8  = EEPROM.read(EEPROM_DISTANCE_LO);
    distance.mid8  = EEPROM.read(EEPROM_DISTANCE_MID);
    distance.high8 = EEPROM.read(EEPROM_DISTANCE_HI);
    Serial.print("distance = ");
    if (distance.high8 < 10) Serial.print("0");
    Serial.print(distance.high8);
    if (distance.mid8 < 10) Serial.print("0");
    Serial.print(distance.mid8);
    if (distance.low8 < 10) Serial.print("0");
    Serial.println(distance.low8);
  }
  distance_pulses = 0;

  read_trip();
}

// reset the trip
void Hodo::reset(void) {
  trip = 0;
  trip_pulses = 0;
  write_trip(0);
  flag_trip = false;
  flag_trip_half = false;
}

// reads the trip fom EEPROM
void Hodo::read_trip(void) {
  byte hi, lo;
  if (fuel.flag_lpg_mode) {
    lo = EEPROM.read(EEPROM_LPG_LO);
    hi = EEPROM.read(EEPROM_LPG_HI);
    Serial.print("LPG ");
  } else {
    lo = EEPROM.read(EEPROM_TRIP_LO);
    hi = EEPROM.read(EEPROM_TRIP_HI);
    Serial.print("unleaded ");
  }
  trip = word(hi, lo);
  trip_pulses = 0;
  Serial.print("trip = ");
  Serial.println(trip);
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
          flag_distance_hi = true;
        }
        flag_distance_mi = true;
      }
      flag_distance_lo = true;
    }
  } 
}

// Write a trip to the EEPROM.
void Hodo::write_trip(unsigned int trip)
{
  byte to_write = lowByte(trip);
  if (fuel.flag_lpg_mode) {
    EEPROM.write(EEPROM_LPG_LO, to_write);
  } else {
    EEPROM.write(EEPROM_TRIP_LO, to_write);
  }

  if (to_write == 0) {
    to_write = highByte(trip);
    if (fuel.flag_lpg_mode) {
      EEPROM.write(EEPROM_LPG_HI, to_write);
    } else {
      EEPROM.write(EEPROM_TRIP_HI, to_write);
    }
  }
}

// Write a distance to the EEPROM.
void Hodo::write_distance(void) {
  // stores distance when it is xxxx.5
  if ((trip_pulses == (pulses_in_100m / 2)) && !flag_trip_half) {
    write_trip(trip + 1);
    // stored. don't store anymore
    flag_trip_half = true;
  }
  if (flag_trip) {
    // ready to store on next half
    flag_trip = false;
    flag_trip_half = false;
  }

  // stores distance when it is xxxx.5
  byte flag_incr = false;
  if ((distance_pulses == 5) && !flag_distance_half) {
    if (distance.low8 >= 100) {
      EEPROM.write(EEPROM_DISTANCE_LO, 0);
      // should increment middle also
      flag_distance_mi = true;
      flag_incr = true;
    } else {
      EEPROM.write(EEPROM_DISTANCE_LO, distance.low8 + 1);
    }
    // stored. don't store anymore
    flag_distance_half = true;
  }
  if (flag_distance_lo) {
    // ready to store on next half
    flag_distance_lo = false;
    flag_distance_half = false;
  }
  if (flag_distance_mi) {
    if (flag_incr) {
      if (distance.mid8 >= 100) {
        EEPROM.write(EEPROM_DISTANCE_MID, 0);
        // should increment high also
        flag_distance_hi = true;
      } else {
        EEPROM.write(EEPROM_DISTANCE_MID, distance.mid8 + 1);
        flag_incr = false;
      }
    } else {
      EEPROM.write(EEPROM_DISTANCE_MID, distance.mid8);
    }
    flag_distance_mi = false;
  }
  if (flag_distance_hi) {
    if (flag_incr) {
      if (distance.mid8 >= 100) {
        EEPROM.write(EEPROM_DISTANCE_HI, 0);
      } else {
        EEPROM.write(EEPROM_DISTANCE_HI, distance.mid8 + 1);
      }
    } else {
      EEPROM.write(EEPROM_DISTANCE_HI, distance.high8);
    }
    flag_distance_hi = false;
  }
}

