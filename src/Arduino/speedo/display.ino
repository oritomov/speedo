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

void Display::init(void) {
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR);  // initialize with the I2C addr (for the 128x64)
  // init done

  // Clear the buffer.
  display.clearDisplay();
  display.display();
}

// translate "speed" to BCD.  Blank leading zeroes, but if num is zero show 0 in ones digit.
void Display::speed(unsigned int speed) {
  unsigned char temp_digit[3];

  // normally I'd use a for loop, but 8 bit division is much faster than 16 bit and we need the speed.
  temp_digit[2] = speed % 10; // ones
  unsigned char temp = speed / 10;
  temp_digit[1] = temp % 10;  // tens
  temp_digit[0] = temp / 10;  // hundreds
  
  display.clearDisplay();
  display.setCursor(30,20);
  display.setTextSize(4);
  display.setTextColor(WHITE);
  if (temp_digit[0] == 0) {
    display.setCursor(50,20);
  } else {
    display.print(temp_digit[0]);
  }
  if ((temp_digit[0] == 0) && (temp_digit[1] == 0)) {
    display.setCursor(70,20);
  } else {
    display.print(temp_digit[1]);
  }
  display.print(temp_digit[2]);
  display.display();
}

void Display::mode(int x, int y, int s, String text) {
  display.clearDisplay();
  display.setTextSize(s);
  display.setTextColor(WHITE);
  display.setCursor(x,y);
  display.print(text);
  display.display();
}

void Display::mode(int x, int y, int s, String text, int p) {
  unsigned char temp_digit[3];
  boolean negative = false;
  if (p < 0) {
    p = -p;
    negative = true;
  }
  // normally I'd use a for loop, but 8 bit division is much faster than 16 bit and we need the speed.
  temp_digit[2] = p % 10; // ones
  unsigned char temp = p / 10;
  temp_digit[1] = temp % 10;  // tens
  temp_digit[0] = temp / 10;  // hundreds
  
  display.clearDisplay();
  display.setTextSize(s);
  display.setTextColor(WHITE);
  display.setCursor(x,y);
  display.print(text);
  if (negative) {
    display.setCursor(30,5);
    display.print("-");
  }
  display.setCursor(30,5);
  if (temp_digit[0] == 0) {
    display.setCursor(50,5);
  } else {
    display.print(temp_digit[0]);
  }
  if ((temp_digit[0] == 0) && (temp_digit[1] == 0)) {
    display.setCursor(70,5);
  } else {
    display.print(temp_digit[1]);
  }
  display.print(temp_digit[2]);
  display.display();
}

