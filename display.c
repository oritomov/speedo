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

#include "display.h"

// ram 
unsigned char sevenseg_bits[13];	// a buffer telling which LEDs we light up on each digit

//set all LEDs on and off
void check_display(void)
{
	// set all LEDs on (lamp test mode) for about 1 second
	while (tmr1_upper < 2)
	{
		sevenseg_bits[12] = sevenseg_bits[11] = sevenseg_bits[10] = 
		sevenseg_bits[9] = sevenseg_bits[8] = sevenseg_bits[7] = 
		sevenseg_bits[6] = sevenseg_bits[5] = sevenseg_bits[4] = sevenseg_bits[3] =
		sevenseg_bits[2] = sevenseg_bits[1] = sevenseg_bits[0] = 0xFF;
	}
	clear_wdt();

	// set all LEDs off (lamp blank mode) for about 0.5 seconds
	while (tmr1_upper < 3)
	{
		sevenseg_bits[12] = sevenseg_bits[11] = sevenseg_bits[10] = 
		sevenseg_bits[9] = sevenseg_bits[8] = sevenseg_bits[7] = 
		sevenseg_bits[6] = sevenseg_bits[5] = sevenseg_bits[4] = sevenseg_bits[3] =
		sevenseg_bits[2] = sevenseg_bits[1] = sevenseg_bits[0] = 0x00;
	}
	clear_wdt();
}

// Looks up digit in the lookup table, then returns the 7 segment bits.
// These shouldn't need to be changed; if your wiring is different, change the SEVSEG macros at the start.
unsigned char sevenseg_lookup(unsigned char digit)
{
	rom unsigned char* lookuptable = {SEVSEG_BITFIELD(0,0,1,1,1,1,1,1), SEVSEG_BITFIELD(0,0,0,0,0,1,1,0),
							SEVSEG_BITFIELD(0,1,0,1,1,0,1,1), SEVSEG_BITFIELD(0,1,0,0,1,1,1,1),
							SEVSEG_BITFIELD(0,1,1,0,0,1,1,0), SEVSEG_BITFIELD(0,1,1,0,1,1,0,1),
							SEVSEG_BITFIELD(0,1,1,1,1,1,0,1), SEVSEG_BITFIELD(0,0,0,0,0,1,1,1),
							SEVSEG_BITFIELD(0,1,1,1,1,1,1,1), SEVSEG_BITFIELD(0,1,1,0,1,1,1,1)  };

	if (digit > 9)
		return 0;
	return lookuptable[digit];
}

// translate "speed" to BCD.  Blank leading zeroes, but if num is zero show 0 in ones digit.
void display_speed(unsigned int speed)
{
	// If you're going faster than 512, send me a video.  I'd like to see it.
	if (speed > 512)
	{
		// TODO sevenseg_text(MSG_EEE);
		return;
	}
	else
	{
		unsigned char temp_digit[3];

		// we need to BCD decode a 9 bit number.

		// normally I'd use a for loop, but 8 bit division is much faster than 16 bit and we need the speed.
		temp_digit[0] = speed % 10;	// ones
		unsigned char temp = speed / 10;
		temp_digit[1] = temp % 10;	// tens
		temp_digit[2] = temp / 10;	// hundreds

		gie = 0;
		sevenseg_bits[2] = 0x00;
		sevenseg_bits[1] = 0x00;

		if (temp_digit[2] != 0)
		{
			//hundreds
			sevenseg_bits[2] = sevenseg_lookup(temp_digit[2]);
			sevenseg_bits[1] = sevenseg_lookup(temp_digit[1]);
		}
		else if (temp_digit[1] != 0)
		{
			//tens
			sevenseg_bits[1] = sevenseg_lookup(temp_digit[1]);
		}
		//ones
		sevenseg_bits[0] = sevenseg_lookup(temp_digit[0]);
		gie = 1;

		// Faster way?  Save the answer and remainder simultaneously, only compute as much as we need.
		// But it would need to be implemented in asm.
	}
}

// translate "trip" to BCD.  Blank leading zeroes, but if num is zero show 0 in ones digit.
void display_trip(unsigned int trip)
{
	if (trip > 9999)
	{
		trip = 0;
	}
	unsigned char temp_digit[4];

	// we need to BCD decode a 9 bit number.

	// normally I'd use a for loop, but 8 bit division is much faster than 16 bit and we need the speed.
	temp_digit[0] = trip % 10;	// meters
	unsigned int temp_uint = trip / 10;
	temp_digit[1] = temp_uint % 10;	// ones
	unsigned char temp_uchar = temp_uint / 10;
	temp_digit[2] = temp_uchar % 10;	// tens
	temp_digit[3] = temp_uchar / 10;	// hundreds

	gie = 0;
	sevenseg_bits[6] = 0x00;
	sevenseg_bits[5] = 0x00;

	if (temp_digit[3] != 0)
	{
		// hundreds
		sevenseg_bits[6] = sevenseg_lookup(temp_digit[3]);
		sevenseg_bits[5] = sevenseg_lookup(temp_digit[2]);
	}
	else if (temp_digit[2] != 0)
	{
		// tens
		sevenseg_bits[5] = sevenseg_lookup(temp_digit[2]);
	}
	// ones
	sevenseg_bits[4] = sevenseg_lookup(temp_digit[1]) | SEVSEG_DECIMAL_MASK;
	// meters
	sevenseg_bits[3] = sevenseg_lookup(temp_digit[0]);
	gie = 1;

		// Faster way?  Save the answer and remainder simultaneously, only compute as much as we need.
		// But it would need to be implemented in asm.
}

// translate "num" to BCD.
void display_distance(uns24 distance)
{
	unsigned char temp_digit[6];

	temp_digit[0] = distance.low8 % 10;	// ones
	temp_digit[1] = distance.low8 / 10;	// tens
	temp_digit[2] = distance.mid8 % 10;	// hundreds
	temp_digit[3] = distance.mid8 / 10;	// tousands
	temp_digit[4] = distance.high8 % 10;	// ten tousands
	temp_digit[5] = distance.high8 / 10;	// hundred tousands

	sevenseg_bits[7] = sevenseg_lookup(temp_digit[0]);	// ones
	sevenseg_bits[8] = sevenseg_lookup(temp_digit[1]);	// tens
	sevenseg_bits[9] = sevenseg_lookup(temp_digit[2]);	// hundreds
	sevenseg_bits[10] = sevenseg_lookup(temp_digit[3]);	// tousands
	sevenseg_bits[11] = sevenseg_lookup(temp_digit[4]);	// ten tousands
	sevenseg_bits[12] = sevenseg_lookup(temp_digit[5]);	// hundred tousands
}
