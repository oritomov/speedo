/*
Display subroutine.
version: 0.1

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



This program should compile under SourceBoost.  Feel free to modify it
to compile with any program you wish, for any platform you wish.
*/

#include "leds.h"

// ram
bit blink_enable;					// set by software: blinking enabled on all displays
unsigned char dim_factor;			// inserts (0xFF - dim_factor) delays in the 7seg refresh to dim the display.
									// if 0xFF, we don't dim displays at all.  If less than that, we dim.
									// (e.g. 0xFC for half power, 0xF9 for 1/3 power, etc)
unsigned char sevenseg_driver[13] = { ones_driver, tens_driver, hundreds_driver,
				trip_100m_driver, trip_ones_driver, trip_tens_driver, trip_hundreds_driver,
				distance_1_driver, distance_2_driver, distance_3_driver,
				distance_4_driver, distance_5_driver, distance_6_driver };

void init_leds(void)
{
	blink_enable = 0;
	dim_factor = 0xFF;

	// tweak pr2 or t2con postscaler to adjust LED refresh rate; too high a refresh means less time for speed calcs
	tmr2 = 0x00;		// reset tmr2
	pr2 = 125;			// wrap when you hit this number
	t2con = 0b00001101;	// ===Start tmr2=== with prescaler 1:4, postscaler 1:2 (pr2 = 125 means interrupt every 1ms)
}

// refreshes a single seven segment element, and increments the digit
// This func is run a lot, so it must be fast!
void refresh_sevenseg(void)
{
	static unsigned char current_digit;

	// turn off all digit driver transistors
	portb &= ~(SEVSEG_PORTB_MASK);
	porta &= ~(SEVSEG_PORTA_MASK);
	SEVSEG_DECIMAL_BIT = 0;

	if (((!blink_enable) || (current_digit >= 3)) || (tmr1h & 0x80))
	{
		// turn on the current digit driver transistor
		porta |= (SEVSEG_PORTA_MASK & sevenseg_driver[current_digit]);
		// assign proper bits to the ports
		portb |= (SEVSEG_PORTB_MASK & sevenseg_bits[current_digit]);
		if (sevenseg_bits[current_digit] & SEVSEG_DECIMAL_MASK)
			SEVSEG_DECIMAL_BIT = 1;	// don't forget decimal
		if (current_digit == 12)
		{
			// normally dim_factor is 0xFF, which means start again at case 0 next pass.
			// if less, we skip lighting up for 0xFF - dim_factor passes.
			// A hack, because current_digit maybe isn't a legal array index...
			current_digit = dim_factor;
		}
	}

	// next pass, we'll update this digit
	current_digit++;
}

// LED interrupt
void interrupt_leds(void)
{
	// tmr2 overflow, refresh next LED cluster - every 1 ms
	if (tmr2if)
	{
		refresh_sevenseg();
		tmr2if = 0;
	}
}
