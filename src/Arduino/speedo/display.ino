/*
Display subroutine.
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

#include "display.h"
#include <Wire.h>

#define EOS 0
#define ZERO 48
#define SPACE 32

AbstractDisplay::AbstractDisplay(byte number): u8g2(number), number(number) {
}

void AbstractDisplay::on(void) {
  Wire.begin(I2C_ADDR);
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(number);
  Wire.endTransmission();
}

void AbstractDisplay::init(void) {
  on();

  u8g2.begin();
}

void AbstractDisplay::display(u8g2_uint_t x, u8g2_uint_t y, const uint8_t *font, const char *str) {
  on();

  u8g2.firstPage();
  do {
    u8g2.setFont(font);
    if (x == 128) {
      x = (128 - u8g2.getStrWidth(str)) / 2;
    }
    u8g2.drawStr(x, y, str);
  } while ( u8g2.nextPage() );
}

BigDisplay::BigDisplay(void): AbstractDisplay(BIG_DISPLAY) {
}

// translate "speed".  Blank leading zeroes, but if num is zero show 0 in ones digit.
void BigDisplay::speed(unsigned int speed) {
  byte x = 4;
  char temp_digit[4];
  byte temp;

  // normally I'd use a for loop, but 8 bit division is much faster than 16 bit and we need the speed.
  temp_digit[3] = EOS;
  temp_digit[2] = (speed % 10) + ZERO; // ones
  temp = speed / 10;
  temp_digit[1] = temp % 10 + ZERO;  // tens
  temp_digit[0] = temp / 10 + ZERO;  // hundreds
  
  if (temp_digit[0] == ZERO) {
    temp_digit[0] = SPACE;
    x += 2;
    if (temp_digit[1] == ZERO) {
      temp_digit[1] = SPACE;
      x += 2;
    }
  }

  display(x, 57 ,u8g2_font_fub49_tn, temp_digit);
}

void BigDisplay::mode(String text) {
  char chr [text.length() + 1];
  text.toCharArray(chr, text.length() + 1);

  display(128, 47, u8g2_font_fub30_tf, chr);
}

SmallDisplay::SmallDisplay(void): AbstractDisplay(SMALL_DISPLAY) {
}

// translate "trip".  Blank not leading zeroes.
void SmallDisplay::trip(unsigned int trip) {
  char temp_digit[6];
  byte temp;

  // normally I'd use a for loop, but 8 bit division is much faster than 16 bit and we need the speed.
  temp_digit[5] = EOS;
  temp_digit[4] = (trip % 10) + ZERO; // meters
  temp_digit[3] = '.';
  temp = trip / 10;
  temp_digit[2] = temp % 10 + ZERO; // ones
  temp = temp / 10;
  temp_digit[1] = temp % 10 + ZERO;  // tens
  temp_digit[0] = temp / 10 + ZERO;  // hundreds

  display(14, 31, u8g2_font_fub30_tf, temp_digit);
}

void SmallDisplay::num(int num) {
  char digit [4];
  sprintf(digit, "%i", num);
  
  display(128, 31, u8g2_font_fub30_tf, digit);
}

