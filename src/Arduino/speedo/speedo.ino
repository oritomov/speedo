/*
Main routine of speedometer for PIC16F648A
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
#include "button.h"
#include "display.h"
#include "fuel.h"
#include "hodo.h"
#include "speed.h"
#include "tires.h"

#define SPEEDOMETER_PIN 2
#define LPG_INPUT       3
#define BUTTON_PIN      4

#define MSG_UNLEAD      "Unlead"
#define MSG_LPG         "LPG"
#define MSG_RESET       "Reset?"
#define MSG_TIRES       "Tires?"
#define MSG_WIDTH       "Width?"
#define MSG_WALL        "Wall?"
#define MSG_WHEEL       "Wheel?"
#define MSG_FIX         "Fix?"
#define MSG_NEXT        "Ok?"
#define MSG_ERROR       "ERROR!"

// The display unit's current task is defined by a state machine.  Normally on boot it defaults to MODE_SPD
// which displays the current speed as a normal speedometer.
// MSG_EEE is not a real mode, but it's here to prevent an overflow bug in sevenseg_text
typedef enum { MODE_EEE,
  MODE_UNL, MODE_LPG,
  MODE_SPD, 
  MODE_MENU_CLR, MODE_CLR,
  MODE_TIRE, 
    MODE_WIDTH, MODE_WALL, MODE_WHEEL, MODE_FIX,
      MODE_NEXT
} mainmode;

#define MODE_DEFAULT  MODE_SPD

// ram 
mainmode current_mode;        // tells our state machine which task the main loop is trying to execute

void setup() {
  Serial.begin(9600);
  bigDisplay.init();
  smallDisplay.init();
  speed.init();
  hodo.init();
  fuel.init();
  button.init();
  
  current_mode = MODE_DEFAULT;
}

int read_tire(int eeprom, int _default, int step, int min) {
  int tire;
  tire = EEPROM.read(eeprom);
  if (tire == 0xFF) {
    tire = _default;
  } else {
    tire *= step;
    tire += min;
  }
  return tire;
}

void calc_tire(void) {
  int width = read_tire(EEPROM_WIDTH, WIDTH_DEFAULT, WIDTH_STEP, WIDTH_MIN);
  int wall = read_tire(EEPROM_WALL, WALL_DEFAULT, WALL_STEP, WALL_MIN);
  int wheel = read_tire(EEPROM_WHEEL, WHEEL_DEFAULT, WHEEL_STEP, WHEEL_MIN);
  int fix = read_tire(EEPROM_FIX, FIX_DEFAULT, FIX_STEP, FIX_MIN);

// (14" * 2.54cm * 0.01 m/cm + 2 * 195mm * 0.001 m/mm * 60%) * pi => 1.85m
// 100m / 1.85m * 8 pulses/turn = 54 turns/100m * 8 pulses/turn = 432 pulses / 100m
  unsigned int pulses_in_100m = (int)(8. * 100. * (1. + (float)fix / 100.) / (((float)wheel * 2.54 * 0.01 + 2. * (float)width * 0.001 * ((float)wall / 100.)) * 3.14));
  Serial.print("pulses in 100m = ");
  Serial.println(pulses_in_100m);
  eeprom_pulses_in_100m(pulses_in_100m);
  hodo.init();
// 3600 sec/h * 1000000 us/sec * 0.1 km / 32 = 11250000 us * 100m / h
// 11250000 / 432 pulses_in_100m = 26041.67
  unsigned int calib_factor = 11250000 / pulses_in_100m;
  Serial.print("calib_factor = ");
  Serial.println(calib_factor);
  eeprom_calib_factor(calib_factor);
  speed.init();
}

void set_mode_and_reset_button(mainmode mode) {
  switch (current_mode) {
    case MODE_SPD:  // calculate and show current speed until a button is held
      Serial.println("Speed");
      break;
    case MODE_UNL:     // show mode
      Serial.println(MSG_UNLEAD);
      break;
    case MODE_LPG:
      Serial.println(MSG_LPG);
      break;
    case MODE_MENU_CLR:   // show menu
      Serial.println(MSG_RESET);
      break;
    case MODE_CLR:      // clear trip
      break;
    case MODE_TIRE:   // show menu
      Serial.println(MSG_TIRES);
    
  }
  current_mode = mode;
  button.reset();
}

void loop() {
  static int current, min, max, step, eeprom;
  boolean lpg;

  // force setup
  if (current_mode != MODE_NEXT) {
    if (EEPROM.read(EEPROM_WIDTH) == 0xFF) {
      set_mode_and_reset_button(MODE_WIDTH);
    } else
    if (EEPROM.read(EEPROM_WALL) == 0xFF) {
      set_mode_and_reset_button(MODE_WALL);
    } else
    if (EEPROM.read(EEPROM_WHEEL) == 0xFF) {
      set_mode_and_reset_button(MODE_WHEEL);
    } else 
    if (EEPROM.read(EEPROM_FIX) == 0xFF) {
      set_mode_and_reset_button(MODE_FIX);
    }
  }

  switch (current_mode) {

    case MODE_SPD:  // calculate and show current speed until a button is held
//      Serial.println("Speed");
        speed.calculate();
        hodo.write_distance();
        bigDisplay.speed(speed.speed);
        smallDisplay.trip(hodo.trip);
        //display.distance(hodo.distance);
        lpg = fuel.flag_lpg_reed;
        if (lpg != fuel.flag_lpg_mode) {
          if (lpg) {
            set_mode_and_reset_button(MODE_LPG);
          } else {
            set_mode_and_reset_button(MODE_UNL);
          }
          fuel.flag_lpg_mode = lpg;
        }
      if (button.status == BUTTON_STATUS_PRESSED) {
        set_mode_and_reset_button(MODE_MENU_CLR);
      }
      break;

    case MODE_UNL:     // show mode
    case MODE_LPG:
      if (current_mode == MODE_UNL) {
        bigDisplay.mode(MSG_UNLEAD);
//        Serial.println(MSG_UNLEAD);
      } else {
        bigDisplay.mode(MSG_LPG);
//        Serial.println(MSG_LPG);
      }
      hodo.read_trip(fuel.flag_lpg_mode);
      switch (button.status) {
        //calculate_speed(0);
        //write_distance();
        //display_trip(trip);
        //display_distance(distance);
        case BUTTON_STATUS_TIMEOUT:
          set_mode_and_reset_button(MODE_DEFAULT);
          break;
      }
      break;

    case MODE_MENU_CLR:   // show menu
//      Serial.println(MSG_RESET);
      bigDisplay.mode(MSG_RESET);
      smallDisplay.trip(hodo.trip);
      switch (button.status) {
        case BUTTON_STATUS_PRESSED:
          set_mode_and_reset_button(MODE_TIRE);
          break;
        case BUTTON_STATUS_HELD:
          set_mode_and_reset_button(MODE_CLR);
          break;
        case BUTTON_STATUS_TIMEOUT:
          set_mode_and_reset_button(MODE_DEFAULT);
          break;
      }
      break;

    case MODE_CLR:      // clear trip
      //Serial.println("Reseting");
      hodo.reset();
      //display_trip(trip);
      set_mode_and_reset_button(MODE_SPD);
      break;

    case MODE_TIRE:   // show menu
//      Serial.println(MSG_TIRES);
      bigDisplay.mode(MSG_TIRES);
      switch (button.status) {
        case BUTTON_STATUS_PRESSED:
          set_mode_and_reset_button(MODE_DEFAULT);    // let's wrap around.
          break;
        case BUTTON_STATUS_HELD:
          set_mode_and_reset_button(MODE_WIDTH);
          break;
        case BUTTON_STATUS_TIMEOUT:
          set_mode_and_reset_button(MODE_DEFAULT);
          break;
      }
      break;

    case MODE_WIDTH:   // show menu
      current = read_tire(EEPROM_WIDTH, WIDTH_DEFAULT, WIDTH_STEP, WIDTH_MIN);
      bigDisplay.mode(MSG_WIDTH);
      smallDisplay.num(current);
      Serial.print(current);
      Serial.println(MSG_WIDTH);
      switch (button.status) {
        case BUTTON_STATUS_PRESSED:
          set_mode_and_reset_button(MODE_WALL);
          break;
        case BUTTON_STATUS_HELD:
          min = WIDTH_MIN;
          max = WIDTH_MAX;
          step = WIDTH_STEP;
          eeprom = EEPROM_WIDTH;
          set_mode_and_reset_button(MODE_NEXT);
          break;
        case BUTTON_STATUS_TIMEOUT:
          set_mode_and_reset_button(MODE_DEFAULT);
          break;
      }
      break;

    case MODE_WALL:   // show menu
      current = read_tire(EEPROM_WALL, WALL_DEFAULT, WALL_STEP, WALL_MIN);
      bigDisplay.mode(MSG_WALL);
      smallDisplay.num(current);
      Serial.print(current);
      Serial.println(MSG_WALL);
      switch (button.status) {
        case BUTTON_STATUS_PRESSED:
          set_mode_and_reset_button(MODE_WHEEL);
          break;
        case BUTTON_STATUS_HELD:
          min = WALL_MIN;
          max = WALL_MAX;
          step = WALL_STEP;
          eeprom = EEPROM_WALL;
          set_mode_and_reset_button(MODE_NEXT);
          break;
        case BUTTON_STATUS_TIMEOUT:
          set_mode_and_reset_button(MODE_DEFAULT);
          break;
      }
      break;

    case MODE_WHEEL:   // show menu
      current = read_tire(EEPROM_WHEEL, WHEEL_DEFAULT, WHEEL_STEP, WHEEL_MIN);
      bigDisplay.mode(MSG_WHEEL);
      smallDisplay.num(current);
      Serial.print(current);
      Serial.println(MSG_WHEEL);
      switch (button.status) {
        case BUTTON_STATUS_PRESSED:
          set_mode_and_reset_button(MODE_FIX);
          break;
        case BUTTON_STATUS_HELD:
          min = WHEEL_MIN;
          max = WHEEL_MAX;
          step = WHEEL_STEP;
          eeprom = EEPROM_WHEEL;
          set_mode_and_reset_button(MODE_NEXT);
          break;
        case BUTTON_STATUS_TIMEOUT:
          set_mode_and_reset_button(MODE_DEFAULT);
          break;
      }
      break;

    case MODE_FIX:   // show menu
      current = read_tire(EEPROM_FIX, FIX_DEFAULT, FIX_STEP, FIX_MIN);
      bigDisplay.mode(MSG_FIX);
      smallDisplay.num(current);
      Serial.print(current);
      Serial.println(MSG_FIX);
      switch (button.status) {
        case BUTTON_STATUS_PRESSED:
          set_mode_and_reset_button(MODE_DEFAULT);    // let's wrap around.
          break;
        case BUTTON_STATUS_HELD:
          min = FIX_MIN;
          max = FIX_MAX;
          step = FIX_STEP;
          eeprom = EEPROM_FIX;
          set_mode_and_reset_button(MODE_NEXT);
          break;
        case BUTTON_STATUS_TIMEOUT:
          set_mode_and_reset_button(MODE_DEFAULT);
          break;
      }
      break;

    case MODE_NEXT:   // show menu
      bigDisplay.mode(MSG_NEXT);
      smallDisplay.num(current);
      Serial.print(current);
      Serial.println(MSG_NEXT);
      switch (button.status) {
        case BUTTON_STATUS_PRESSED:
          current += step;
          if (current > max) {
            current = min;
          }
          set_mode_and_reset_button(MODE_NEXT);
          break;
        case BUTTON_STATUS_HELD:
          current -= min;
          current /= step;
          write(eeprom, (byte)current);
          calc_tire();
          set_mode_and_reset_button(MODE_DEFAULT);
          break;
        case BUTTON_STATUS_TIMEOUT:
          set_mode_and_reset_button(MODE_DEFAULT);
          break;
      }
      break;

    default:
      bigDisplay.mode(MSG_ERROR);
      Serial.println(MSG_ERROR);
      // We should never arrive here.
      // if we're debugging, we want to know about this; print MSG_EEE and lockup.
      while (button.status == BUTTON_STATUS_NONE) {
      }
      // if we're not debugging, just get us back into MODE_DEFAULT.
      set_mode_and_reset_button(MODE_DEFAULT);
      break;
  }
}
