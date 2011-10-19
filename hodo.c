/*
Hodo subroutine.
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

#include "hodo.h"

// eeprom
#define eeprom_pulses_in_100m_lo    2
#define eeprom_pulses_in_100m_hi    3

#define eeprom_distance_lo          4
#define eeprom_distance_mid         5
#define eeprom_distance_hi          6

#define eeprom_trip_lo              7
#define eeprom_trip_hi              8
#define eeprom_lpg_lo               9
#define eeprom_lpg_hi              10

// ram
unsigned int pulses_in_100m;
uns24 distance;
char distance_pulses;
unsigned int trip;
unsigned int trip_pulses;
// flags determine storing to eprom
bit flag_distance_lo;
bit flag_distance_mi;
bit flag_distance_hi;
bit flag_distance_half;
bit flag_trip;
bit flag_trip_half;
bit flag_lpg_reed;                  // use for lpg reed signal.
bit flag_lpg_mode;                  // use for unleaded or lpg mode.

uns24 distance(void)
{
	return distance;
}

void clear_trip()
{
	trip = 0;
	write_trip(0);
	trip_pulses = 0;
}

unsigned int trip(void)
{
	return trip;
}

void init_hodo(void)
{
    flag_distance_lo = 0; // init
    flag_distance_mi = 0;
    flag_distance_hi = 0;
    flag_distance_half = 0;
    flag_trip = 0;
    flag_trip_half = 0;
    flag_lpg_reed = 0;
    flag_lpg_mode = 0;

    // pulses_in_100m
    {
        unsigned char temp1, temp2;
        temp1 = eeprom_read(eeprom_pulses_in_100m_lo);
        temp2 = eeprom_read(eeprom_pulses_in_100m_hi);
        MAKESHORT(pulses_in_100m, temp1, temp2);
    }

    // distance
    {
        distance.low8  = eeprom_read(eeprom_distance_lo);
        distance.mid8  = eeprom_read(eeprom_distance_mid);
        distance.high8 = eeprom_read(eeprom_distance_hi);
    }
    distance_pulses = 0;
    
    read_trip();
}

// increments distance on every ten 100m
void read_trip(void)
{
	if (flag_lpg_mode)
	{
        unsigned char temp1, temp2;
        temp1 = eeprom_read(eeprom_lpg_lo);
        temp2 = eeprom_read(eeprom_lpg_hi);
        MAKESHORT(trip, temp1, temp2);
	}
	else
	{
        unsigned char temp1, temp2;
        temp1 = eeprom_read(eeprom_trip_lo);
        temp2 = eeprom_read(eeprom_trip_hi);
        MAKESHORT(trip, temp1, temp2);
	}
    trip_pulses = 0;
}

// increments distance on every 10 100m
void incr_distance(void)
{
    trip_pulses++;
    if (trip_pulses >= pulses_in_100m)
    {
        trip_pulses = 0;
        trip++;
        if (trip > 9999)
        {
            trip = 0;
        }
        flag_trip = 1; // write to eprom

        distance_pulses++;
        if (distance_pulses >= 10)
        {
            distance_pulses = 0;
            distance.low8++;
            if (distance.low8 >= 100)
            {
                distance.low8 = 0;
                distance.mid8++;
                if (distance.mid8 >= 100)
                {
                    distance.mid8 = 0;
                    distance.high8++;
                    if (distance.high8 >= 100)
                    {
                        distance.high8 = 0;
                    }
                    flag_distance_hi = 1;
                }
                flag_distance_mi = 1;
            }
            flag_distance_lo = 1;
        }
    } 
}

// Write a distance to the EEPROM.
void write_trip(unsigned int trip)
{
    unsigned char to_write = 0xFF;
    LOBYTE(to_write, trip);
	if (flag_lpg_mode)
        eeprom_write(eeprom_lpg_lo, to_write);
    else
        eeprom_write(eeprom_trip_lo, to_write);

    if (to_write == 0)
    {
        HIBYTE(to_write, trip);
    	if (flag_lpg_mode)
            eeprom_write(eeprom_lpg_hi, to_write);
	    else
            eeprom_write(eeprom_trip_hi, to_write);
    }
}

// Write a distance to the EEPROM.
void write_distance(void)
{
    // stores distance when it is xxxx.5
    if ((trip_pulses == (pulses_in_100m / 2)) && !flag_trip_half)
    {
    	write_trip(trip + 1);
        // stored. don't store anymore
        flag_trip_half = 1;
    }
    if (flag_trip)
    {
        // ready to store on next half
        flag_trip = 0;
        flag_trip_half = 0;
    }

    // stores distance when it is xxxx.5
    bit flag_incr = 0;
    if ((distance_pulses == 5) && !flag_distance_half)
    {
    	if (distance.low8 >= 100)
    	{
    		eeprom_write(eeprom_distance_lo, 0);
    		// should increment middle also
    		flag_distance_mi = 1;
    		flag_incr = 1;
    	}
    	else
            eeprom_write(eeprom_distance_lo, distance.low8 + 1);
        // stored. don't store anymore
        flag_distance_half = 1;
    }
    if (flag_distance_lo)
    {
        // ready to store on next half
        flag_distance_lo = 0;
        flag_distance_half = 0;
    }
    if (flag_distance_mi)
    {
    	if (flag_incr)
    	{
    	    if (distance.mid8 >= 100)
    	    {
    		    eeprom_write(eeprom_distance_mid, 0);
    		    // should increment high also
    		    flag_distance_hi = 1;
    	    }
    	    else
    	    {
                eeprom_write(eeprom_distance_mid, distance.mid8 + 1);
    		    flag_incr = 0;
            }
		}
		else
            eeprom_write(eeprom_distance_mid, distance.mid8);
        flag_distance_mi = 0;
    }
    if (flag_distance_hi)
    {
    	if (flag_incr)
    	{
    	    if (distance.mid8 >= 100)
    		    eeprom_write(eeprom_distance_hi, 0);
    	    else
                eeprom_write(eeprom_distance_hi, distance.mid8 + 1);
		}
		else
            eeprom_write(eeprom_distance_hi, distance.high8);
        flag_distance_hi = 0;
    }
}

void interrupt_lpg(void)
{
    if (!LPG_INPUT)  // if LPG is on, LPG_INPUT is held low.
    {
        flag_lpg_reed = 1;
    }
    else
    {
        flag_lpg_reed = 0;
    }
}
