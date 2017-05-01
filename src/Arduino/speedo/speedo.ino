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

// The display unit's current task is defined by a state machine.  Normally on boot it defaults to MODE_SPD
// which displays the current speed as a normal speedometer.
// MSG_EEE is not a real mode, but it's here to prevent an overflow bug in sevenseg_text
typedef enum { MSG_EEE,
  MSG_UNL, MSG_LPG,
  MODE_SPD, 
  MODE_MENU_CLR, MODE_CLR,
#ifdef tires
  MODE_TIRE, 
    MODE_WIDTH, MODE_WALL, MODE_WHEEL, MODE_FIX,
      MODE_NEXT
#endif //tires
} mainmode;

#define MODE_DEFAULT  MODE_SPD

// ram 
mainmode current_mode;        // tells our state machine which task the main loop is trying to execute

void setup() {
  Serial.begin(9600);
  display.init();
  speed.init(SPEEDOMETER_PIN);
  hodo.init();
  fuel.init(LPG_INPUT);
  button.init(BUTTON_PIN);
  
  current_mode = MODE_DEFAULT;

  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A); // Enable Output Compare Match A Interrupt
}

// Interrupt is called once a millisecond, 
SIGNAL(TIMER0_COMPA_vect) {
  button.interrupt();
}

#ifdef tires
int read_tire(int eeprom, int _default, int step, int min) {
  int tire;
  tire = EEPROM.read(eeprom);
  if (tire = 0xFF) {
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
// 3600 sec/h * 1000000 us/sec * 0.1 km / 128 = 2812500 us * 100m / h
// 2812500 / 432 pulses_in_100m = 6510
  unsigned int calib_factor = 2812500 / pulses_in_100m;
  Serial.print("calib_factor = ");
  Serial.println(calib_factor);
}
#endif //tires

void loop() {
  static int current, min, max, step, eeprom;
  unsigned long previousMillis = millis();

#ifdef tires
  // force setup
  if (current_mode != MODE_NEXT) {
    if (EEPROM.read(EEPROM_WIDTH) == 0xFF) {
      current_mode = MODE_WIDTH;
    } else
    if (EEPROM.read(EEPROM_WALL) == 0xFF) {
      current_mode = MODE_WALL;
    } else
    if (EEPROM.read(EEPROM_WHEEL) == 0xFF) {
      current_mode = MODE_WHEEL;
    } else 
    if (EEPROM.read(EEPROM_FIX) == 0xFF) {
      current_mode = MODE_FIX;
    }
  }
#endif //tires

  button.reset();

  switch (current_mode) {

    case MODE_SPD:  // calculate and show current speed until a button is held
      Serial.println("Speed");
      while (!button.pressed) {
        speed.calculate();
        hodo.write_distance();
        display.speed(speed.speed);
        //display_trip(trip);
        //display_distance(distance);
        boolean lpg = fuel.flag_lpg_reed;
        if (lpg != fuel.flag_lpg_mode) {
          if (lpg) {
            current_mode = MSG_LPG;
          } else {
            current_mode = MSG_UNL;
          }
          fuel.flag_lpg_mode = lpg;
          break;
        }
      }
      // switch to first menu state
      if (current_mode == MODE_SPD)
        current_mode = MODE_MENU_CLR;
      break;

    case MSG_UNL:     // show mode
    case MSG_LPG:
      if (current_mode == MSG_UNL) {
        display.mode(15, 20, 3, "Unlead");
        Serial.println("Unlead");
      } else {
        display.mode(50, 20, 3, "LPG");
        Serial.println("LPG");
      }
      hodo.read_trip();
      while (true) {
        //calculate_speed(0);
        //write_distance();
        //display_trip(trip);
        //display_distance(distance);
        if (millis() - previousMillis > 5000) {
          current_mode = MODE_DEFAULT;
          break;
        }
      }
      current_mode = MODE_SPD;
      break;

    case MODE_MENU_CLR:   // show menu
      Serial.println("Reset?");
      //display_mode(15, 20, 3, "Reset?");
      display.mode(15, 40, 3, "Reset?", hodo.trip /10);
      while (true) {

        if (button.pressed) {
#ifdef tires
          current_mode = MODE_TIRE;
#else //tires
          current_mode = MODE_DEFAULT;    // let's wrap around.
#endif //tires
          break;
        } else if (button.held) {
          current_mode = MODE_CLR;
          break;
        }
        if (millis() - previousMillis > 5000) {
          current_mode = MODE_DEFAULT;
          break;
        }
      }
      break;

    case MODE_CLR:      // clear trip
      Serial.println("Reseting");
      hodo.reset();
      //display_trip(trip);
      current_mode = MODE_SPD;
      break;

#ifdef tires
    case MODE_TIRE:   // show menu
      Serial.println("Tires?");
      display.mode(15, 20, 3, "Tires?");
      while (true) {
        if (button.pressed) {
          current_mode = MODE_DEFAULT;    // let's wrap around.
          break;
        } else if (button.held) {
          current_mode = MODE_WIDTH;
          break;
        }
        if (millis() - previousMillis > 5000) {
          current_mode = MODE_DEFAULT;
          break;
        }
      }
      break;

    case MODE_WIDTH:   // show menu
      current = read_tire(EEPROM_WIDTH, WIDTH_DEFAULT, WIDTH_STEP, WIDTH_MIN);
      display.mode(15, 40, 3, "Width?", current);
      Serial.print(current);
      Serial.println(" Width?");
      while (true) {

        if (button.pressed) {
          current_mode = MODE_WALL;
          break;
        } else if (button.held) {
          min = WIDTH_MIN;
          max = WIDTH_MAX;
          step = WIDTH_STEP;
          eeprom = EEPROM_WIDTH;
          current_mode = MODE_NEXT;
          break;
        }
        if (millis() - previousMillis > 5000) {
          current_mode = MODE_DEFAULT;
          break;
        }
      }
      break;

    case MODE_WALL:   // show menu
      current = read_tire(EEPROM_WALL, WALL_DEFAULT, WALL_STEP, WALL_MIN);
      display.mode(25, 40, 3, "Wall?", current);
      Serial.print(current);
      Serial.println(" Wall?");
      while (true) {

        if (button.pressed) {
          current_mode = MODE_WHEEL;
          break;
        } else if (button.held) {
          min = WALL_MIN;
          max = WALL_MAX;
          step = WALL_STEP;
          eeprom = EEPROM_WALL;
          current_mode = MODE_NEXT;
          break;
        }
        if (millis() - previousMillis > 5000) {
          current_mode = MODE_DEFAULT;
          break;
        }
      }
      break;

    case MODE_WHEEL:   // show menu
      current = read_tire(EEPROM_WHEEL, WHEEL_DEFAULT, WHEEL_STEP, WHEEL_MIN);
      display.mode(15, 40, 3, "Wheel?", current);
      Serial.print(current);
      Serial.println(" Wheel?");
      while (true) {

        if (button.pressed) {
          current_mode = MODE_FIX;
          break;
        } else if (button.held) {
          min = WHEEL_MIN;
          max = WHEEL_MAX;
          step = WHEEL_STEP;
          eeprom = EEPROM_WHEEL;
          current_mode = MODE_NEXT;
          break;
        }
        if (millis() - previousMillis > 5000) {
          current_mode = MODE_DEFAULT;
          break;
        }
      }
      break;

    case MODE_FIX:   // show menu
      current = read_tire(EEPROM_FIX, FIX_DEFAULT, FIX_STEP, FIX_MIN);
      display.mode(35, 40, 3, "Fix?", current);
      Serial.print(current);
      Serial.println(" Fix?");
      while (true) {

        if (button.pressed) {
          current_mode = MODE_DEFAULT;    // let's wrap around.
          break;
        } else if (button.held) {
          min = FIX_MIN;
          max = FIX_MAX;
          step = FIX_STEP;
          eeprom = EEPROM_FIX;
          current_mode = MODE_NEXT;
          break;
        }
        if (millis() - previousMillis > 5000) {
          current_mode = MODE_DEFAULT;
          break;
        }
      }
      break;

    case MODE_NEXT:   // show menu
      display.mode(50, 40, 3, "Ok?", current);
      Serial.print(current);
      Serial.println(" Ok?");
      while (true) {

        if (button.pressed) {
          current += step;
          if (current > max) {
            current = min;
          }
          current_mode = MODE_NEXT;
          break;
        } else if (button.held) {
          current -= min;
          current /= step;
          EEPROM.write(eeprom, current);
          calc_tire();
          current_mode = MODE_DEFAULT;
          break;
        }
        if (millis() - previousMillis > 5000) {
          current_mode = MODE_DEFAULT;
          break;
        }
      }
      break;
#endif //tires

    default:
      display.mode(15, 20, 3, "ERROR!");
      Serial.println("ERROR!");
      // We should never arrive here.
      // if we're debugging, we want to know about this; print MSG_EEE and lockup.
      while (!button.pressed)
        ;
      // if we're not debugging, just get us back into MODE_DEFAULT.
      current_mode = MODE_DEFAULT;
  }
}

