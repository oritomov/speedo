/*
Speed calculation subroutine.
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

KNOWN BUGS:

Decreases in speed register immediately and are not affected by averaging.
This is deliberate; otherwise, speed decreases are not registered quickly
and accurately.  However, this can cause the display to oscillate rapidly
between two speeds, for example 38 and 40.
*/

// (14" * 2.54cm * 0.01 m/cm + 2 * 195mm * 0.001 m/mm * 60%) * pi => 1.85m
// 100m / 1.85m * 8 pulses/turn = 54 turns/100m * 8 pulses/turn = 432 pulses / 100m
//#define DEFAULT_PULSES_IN_100M      432
// 3600 sec/h * 1000000 us/sec * 0.1 km / 32 = 11250000 us * 100m / h
// 11250000 / 432 pulses_in_100m = 26041.66666666667
//#define DEFAULT_CALIB_FACTOR  26042

void Speed::init(void) {
  calib_factor = eeprom_calib_factor();

  lastupdatedtime = 0;
  for (uint8_t i = 0; i < 6; i++)
    rollingsum[i] = 0;
  lastpulsecount = 0;
}

// calculate our speed and write it to the "speed" variable
// "forceupdate" means ignore hysteresis and return raw speed
void Speed::calculate(boolean forceupdate) {
	uint16_t twopulse_delta, lastpulse_delta, temp_lastpulses[2];

	int newspeed;	// signed int

	// if we don't have two valid pulses, we're done. Speed is zero.
	if (hall.stale()) {
		for (uint8_t i = 0; i < 6; i++)
			rollingsum[i] = 0;	// clear all rolling sums
		speed = 0;
		return;
	}

  uint16_t now = micros() >> 5;
  hall.get_lastpulses(temp_lastpulses);

  lastpulse_delta = now;

	// calculate difference between last 2 pulses' clock
	twopulse_delta = temp_lastpulses[0] - temp_lastpulses[1];
	// calculate difference between last pulse and now
	// lastpulse_delta = now - temp_lastpulses[0];
	lastpulse_delta -= temp_lastpulses[0];

	if (lastpulsecount != hall.pulsecount) {  // NOTE: only update rolling average of times between two pulses
		// we have a new pulse so we can update antijitter
		lastpulsecount = hall.pulsecount;
		// rollingsum[n] = ((2^n - 1) * rollingsum[n] / (2^n)) + (twopulse_delta)
		uint8_t i;
		for (i = 0; i < 6; i++) {
			rollingsum[i] -= (rollingsum[i] >> (i+1));
			rollingsum[i] += twopulse_delta;
		}
	}

	// whichever of those is greater, call that "period"
	// speed is calibfactor / period
	if (twopulse_delta > lastpulse_delta) {
		newspeed = calib_factor / twopulse_delta;
	} else if (lastpulse_delta == 0) {  // if (lastpulse_delta==0) && (twopulse_delta==0) catch the edge case so we don't divide by zero
		newspeed = 0;
	} else {  // in this case, our speed is lower because we haven't got a pulse yet as expected...
		newspeed = calib_factor / lastpulse_delta;
	}

	// n = calib_factor / (twopulse_delta * 10)
	uint8_t n = (uint16_t)newspeed / 10;

	if (n) {  // if going faster than 10kph, use an appropriate rolling average
		uint8_t i = 0;	// "i" will tell us how many speed estimates to blend (2 for 10kph, 4 for 20, 8 for 40, and so on)
		while (n) {
			n = n >> 1;
			i++;
		}
		uint16_t sumpulse = (rollingsum[i-1] >> i);
		if (sumpulse) { // if (rollingsum[i-1] >> i) gets glitched to be zero, we'd be in serious trouble
			newspeed = calib_factor / sumpulse;
		}
	}

	int t = (int)newspeed - speed;
	// we don't want to update the display if it hasn't changed much and it was already updated recently (within the last 0.25sec)
	if ((2 > t) && (t > -2) && (lastupdatedtime == (highByte(now) & 0x80)) && !forceupdate) {
		return;
	} else {
		lastupdatedtime = highByte(now) & 0x80;
		speed = (uint8_t)newspeed;
	}
}

