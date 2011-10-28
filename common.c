/*
Commom calculation subroutine.
Thanks to Jeff Hiner
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

#include "common.h"

// turns a 24 bit "long timer" entry into a 16 bit "short timer"
// equivalent to (word)(bigtimer >> 2)
unsigned int make16(uns24 bigtimer)
{
	unsigned int rval;
	// roll high8, mid8, low8; do it twice
	asm {
		rrf _bigtimer+2, 1
		rrf _bigtimer+1, 1
		rrf _bigtimer, 1
		rrf _bigtimer+2, 1
		rrf _bigtimer+1, 1
		rrf _bigtimer, 1
	}
	MAKESHORT(rval, bigtimer, bigtimer+1);
	return rval;
}

// turns a 24 bit "long timer" entry into a 16 bit "short timer"
// (Use this one inside interrupts to avoid static memory limitations)
unsigned int make16i(uns24 bigtimer)
{
	unsigned int rval;
	asm {
		rrf _bigtimer+2, 1
		rrf _bigtimer+1, 1
		rrf _bigtimer, 1
		rrf _bigtimer+2, 1
		rrf _bigtimer+1, 1
		rrf _bigtimer, 1
	}
	MAKESHORT(rval, bigtimer, bigtimer+1);
	return rval;
}

unsigned char eeprom_read(unsigned char address)
{
	eeadr = address;
	eecon1.RD = 1;
	return eedata;
}

// Write a byte of data to the given EEPROM address.
// This function will block execution if a previous write hasn't finished yet.
void eeprom_write(unsigned char address, unsigned char data)
{
	while (eecon1.WR)
		;
	eeadr = address;
	eedata = data;
	gie = 0;
	eecon1.WREN = 1;	// enable writes
	// write sequence
	eecon2 = 0x55;
	eecon2 = 0xAA;
	eecon1.WR = 1;

	eecon1.WREN = 0;	// disable writes
	gie = 1;

}
