/*
Read/write eeprom subroutine.
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
#include "eeprom.h"

#define EEPROM_CALIB_FACTOR_KPH_LO  0
#define EEPROM_CALIB_FACTOR_KPH_HI  1

#define EEPROM_PULSES_IN_100M_LO  2
#define EEPROM_PULSES_IN_100M_HI  3

#define EEPROM_DISTANCE_LO      4
#define EEPROM_DISTANCE_MID     5
#define EEPROM_DISTANCE_HI      6

#define EEPROM_TRIP_LO        7
#define EEPROM_TRIP_HI        8
#define EEPROM_LPG_LO       9
#define EEPROM_LPG_HI       10

void write(byte address, byte value) {
  Serial.print(address);
  Serial.print(" : ");
  Serial.println(value);
  EEPROM.write(address, value);
}

unsigned int eeprom_calib_factor(void) {
  byte hi, lo;
  hi = EEPROM.read(EEPROM_CALIB_FACTOR_KPH_HI);
  lo = EEPROM.read(EEPROM_CALIB_FACTOR_KPH_LO);
  unsigned int calib_factor = word(hi, lo);
  Serial.print("calib_factor = ");
  Serial.println(calib_factor);
  return calib_factor;
//    if (calib_factor == 0xffff)
//      calib_factor = DEFAULT_CALIB_FACTOR;
}

void eeprom_calib_factor(unsigned int calib_factor) {
    byte hi = highByte(calib_factor);
    byte lo = lowByte(calib_factor);
    write(EEPROM_CALIB_FACTOR_KPH_HI, hi);
    write(EEPROM_CALIB_FACTOR_KPH_LO, lo);
}

unsigned int eeprom_pulses_in_100m(void) {
  byte hi, lo;
  hi = EEPROM.read(EEPROM_PULSES_IN_100M_HI);
  lo = EEPROM.read(EEPROM_PULSES_IN_100M_LO);
  unsigned int pulses_in_100m = word(hi, lo);
  Serial.print("pulses_in_100m = ");
  Serial.println(pulses_in_100m);
  return pulses_in_100m;
//    if (pulses_in_100m == 0xffff)
//      pulses_in_100m = DEFAULT_PULSES_IN_100M;
}

void eeprom_pulses_in_100m(unsigned int pulses_in_100m) {
  byte hi = highByte(pulses_in_100m);
  byte lo = lowByte(pulses_in_100m);
  write(EEPROM_PULSES_IN_100M_HI, hi);
  write(EEPROM_PULSES_IN_100M_LO, lo);
}

_uns24 eeprom_distance(void) {
  _uns24 distance;
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
  return distance;
}

// reads the trip fom EEPROM
unsigned int eeprom_trip(boolean flag_lpg_mode) {
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
  unsigned int trip = word(hi, lo);
  Serial.print("trip = ");
  Serial.println(trip);
  return trip;
}

// Write a trip to the EEPROM.
void eeprom_trip(unsigned int trip, boolean flag_lpg_mode) {
  //Serial.print("Hodo::write_trip ");
  //Serial.println(trip);
  byte to_write = lowByte(trip);
  //Serial.print("to_write low ");
  //Serial.println(to_write);
  if (flag_lpg_mode) {
    write(EEPROM_LPG_LO, to_write);
  } else {
    write(EEPROM_TRIP_LO, to_write);
  }

  if (to_write == 0) {
    to_write = highByte(trip);
    //Serial.print("to_write high ");
    //Serial.println(to_write);
    if (flag_lpg_mode) {
      write(EEPROM_LPG_HI, to_write);
    } else {
      write(EEPROM_TRIP_HI, to_write);
    }
  }
}

void eeprom_distance(_uns24 distance, boolean threshold, flags *flag_distance) {
  byte flag_incr = false;
  if (threshold && !flag_distance->half) {
    if (distance.low8 >= 100) {
      write(EEPROM_DISTANCE_LO, 0);
      // should increment middle also
      flag_distance->mi = true;
      flag_incr = true;
    } else {
      write(EEPROM_DISTANCE_LO, distance.low8 + 1);
    }
    // stored. don't store anymore
    flag_distance->half = true;
  }
  if (flag_distance->lo) {
    // ready to store on next half
    flag_distance->lo = false;
    flag_distance->half = false;
  }
  if (flag_distance->mi) {
    if (flag_incr) {
      if (distance.mid8 >= 100) {
        write(EEPROM_DISTANCE_MID, 0);
        // should increment high also
        flag_distance->hi = true;
      } else {
        write(EEPROM_DISTANCE_MID, distance.mid8 + 1);
        flag_incr = false;
      }
    } else {
      write(EEPROM_DISTANCE_MID, distance.mid8);
    }
    flag_distance->mi = false;
  }
  if (flag_distance->hi) {
    if (flag_incr) {
      if (distance.mid8 >= 100) {
        write(EEPROM_DISTANCE_HI, 0);
      } else {
        write(EEPROM_DISTANCE_HI, distance.mid8 + 1);
      }
    } else {
      write(EEPROM_DISTANCE_HI, distance.high8);
    }
    flag_distance->hi = false;
  }
}

