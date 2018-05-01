/*
Read/write eeprom & common subroutine.
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

// turns a 24 bit "long timer" entry into a 16 bit "short timer"
// equivalent to (word)(bigtimer)
uint16_t make16(uint24_t bigtimer) {
  uint16_t val;
  val = word(bigtimer.mid8, bigtimer.low8);
  return val;
}

// Read a byte of data from the given EEPROM address.
uint8_t eeprom_read(uint8_t address) {
  uint8_t data;
  data = EEPROM.read(address);
//  Serial.print(address);
//  Serial.print(" : ");
//  Serial.println(data);
  return data;
}

// Write a byte of data to the given EEPROM address.
void eeprom_write(uint8_t address, uint8_t data) {
  Serial.print(address);
  Serial.print(" : ");
  Serial.println(data);
  EEPROM.write(address, data);
}

// Read a word of data from the given EEPROM address.
uint16_t eeprom_read(uint8_t addressHi, uint8_t addressLo) {
  uint8_t hi, lo;
  hi = eeprom_read(addressHi);
  lo = eeprom_read(addressLo);
  uint16_t data = word(hi, lo);
  return data;
}

// Write a word of data to the given EEPROM address.
void eeprom_write(uint8_t addressHi, uint8_t addressLo, uint16_t data) {
  uint8_t hi = highByte(data);
  uint8_t lo = lowByte(data);
  eeprom_write(addressHi, hi);
  eeprom_write(addressLo, lo);
}

uint16_t eeprom_calib_factor(void) {
  uint16_t calib_factor = eeprom_read(EEPROM_CALIB_FACTOR_KPH_HI, EEPROM_CALIB_FACTOR_KPH_LO);
//  if (calib_factor == 0xffff)
//    calib_factor = DEFAULT_CALIB_FACTOR;
  Serial.print("calib_factor = ");
  Serial.println(calib_factor);
  return calib_factor;
}

void eeprom_calib_factor(uint16_t calib_factor) {
  eeprom_write(EEPROM_CALIB_FACTOR_KPH_HI, EEPROM_CALIB_FACTOR_KPH_LO, calib_factor);
}

uint16_t eeprom_pulses_in_100m(void) {
  uint16_t pulses_in_100m =  eeprom_read(EEPROM_PULSES_IN_100M_HI, EEPROM_PULSES_IN_100M_LO);
//  if (pulses_in_100m == 0xffff)
//    pulses_in_100m = DEFAULT_PULSES_IN_100M;
  Serial.print("pulses_in_100m = ");
  Serial.println(pulses_in_100m);
  return pulses_in_100m;
}

void eeprom_pulses_in_100m(uint16_t pulses_in_100m) {
  eeprom_write(EEPROM_PULSES_IN_100M_HI, EEPROM_PULSES_IN_100M_LO, pulses_in_100m);
}

// reads the trip fom EEPROM
uint16_t eeprom_trip(boolean flag_lpg_mode) {
  uint16_t trip;
  if (flag_lpg_mode) {
    trip = eeprom_read(EEPROM_LPG_HI, EEPROM_LPG_LO);
    Serial.print("LPG ");
  } else {
    trip = eeprom_read(EEPROM_TRIP_HI, EEPROM_TRIP_LO);
    Serial.print("unleaded ");
  }
  Serial.print("trip = ");
  Serial.println(trip);
  return trip;
}

// Write a trip to the EEPROM.
void eeprom_trip(uint16_t trip, boolean flag_lpg_mode) {
  //Serial.print("Hodo::write_trip ");
  //Serial.println(trip);
  uint8_t to_write = lowByte(trip);
  //Serial.print("to_write low ");
  //Serial.println(to_write);
  if (flag_lpg_mode) {
    eeprom_write(EEPROM_LPG_LO, to_write);
  } else {
    eeprom_write(EEPROM_TRIP_LO, to_write);
  }

  if (to_write == 0) {
    to_write = highByte(trip);
    //Serial.print("to_write high ");
    //Serial.println(to_write);
    if (flag_lpg_mode) {
      eeprom_write(EEPROM_LPG_HI, to_write);
    } else {
      eeprom_write(EEPROM_TRIP_HI, to_write);
    }
  }
}

uint24_t eeprom_distance(void) {
  uint24_t distance;
  distance.low8  = eeprom_read(EEPROM_DISTANCE_LO);
  distance.mid8  = eeprom_read(EEPROM_DISTANCE_MID);
  distance.high8 = eeprom_read(EEPROM_DISTANCE_HI);
  Serial.print("distance = ");
  if (distance.high8 < 10) Serial.print("0");
  Serial.print(distance.high8);
  if (distance.mid8 < 10) Serial.print("0");
  Serial.print(distance.mid8);
  if (distance.low8 < 10) Serial.print("0");
  Serial.println(distance.low8);
  return distance;
}

void eeprom_distance(uint24_t distance, boolean threshold, flags *flag_distance) {
  boolean flag_incr = false;
  if (threshold && !flag_distance->half) {
    if (distance.low8 >= 100) {
      eeprom_write(EEPROM_DISTANCE_LO, 0);
      // should increment middle also
      flag_distance->mi = true;
      flag_incr = true;
    } else {
      eeprom_write(EEPROM_DISTANCE_LO, distance.low8 + 1);
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
        eeprom_write(EEPROM_DISTANCE_MID, 0);
        // should increment high also
        flag_distance->hi = true;
      } else {
        eeprom_write(EEPROM_DISTANCE_MID, distance.mid8 + 1);
        flag_incr = false;
      }
    } else {
      eeprom_write(EEPROM_DISTANCE_MID, distance.mid8);
    }
    flag_distance->mi = false;
  }
  if (flag_distance->hi) {
    if (flag_incr) {
      if (distance.mid8 >= 100) {
        eeprom_write(EEPROM_DISTANCE_HI, 0);
      } else {
        eeprom_write(EEPROM_DISTANCE_HI, distance.mid8 + 1);
      }
    } else {
      eeprom_write(EEPROM_DISTANCE_HI, distance.high8);
    }
    flag_distance->hi = false;
  }
}


