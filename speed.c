/*
Speed calculation subroutine.
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

KNOWN BUGS:

Decreases in speed register immediately and are not affected by averaging.
This is deliberate; otherwise, speed decreases are not registered quickly
and accurately.  However, this can cause the display to oscillate rapidly
between two speeds, for example 38 and 40.
*/

#include "speed.h"

// eeprom
#define eeprom_calib_factor_kph_lo  0
#define eeprom_calib_factor_kph_hi  1

// ram
unsigned int calib_factor;          // calibration factor used to calculate speed
unsigned int speed;                 // our current speed, calculated in main loop (unsigned char only goes to 255)
unsigned int pulsecount;            // updated on each pulse.  Used for drag timing.
uns24 lastpulses[2];                // the last two pulses received.  index 0 is last, index 1 is the one before that.
bit lastpulse_stale0;
bit lastpulse_stale1;               // if a pulse happened too long ago, it's stale and we can't do anything with it
unsigned char tmr1_upper;           // since 0.5 seconds isn't long enough, we increment this with tmr1 overflows
                                    //   and use it for top byte of lastpulses.
uns24 now;                          // last sampled tmr1/tmr1_upper "now" time.

void init_speed(void)
{
    // calib_factor
    {
        unsigned char temp1, temp2;
        temp1 = eeprom_read(eeprom_calib_factor_kph_lo);
        temp2 = eeprom_read(eeprom_calib_factor_kph_hi);
        MAKESHORT(calib_factor, temp1, temp2);
    }

    lastpulse_stale0 = 1;   // both pulses are invalid on reset; we need to get TWO pulses in a reasonable time
    lastpulse_stale1 = 1;   // before doing any calculating.

    now.low8 = 0;
    now.mid8 = 0;
    now.high8 = 0;

    tmr1l = 0x00;           // reset tmr1
    tmr1h = 0x00;
    tmr1_upper = 0;
    ccpr1l = 0x00;          // and CCP cap timer
    ccpr1h = 0x00;
    t1con = 0b00110001;     // ===Start tmr1=== with 1:8 prescale; 125000 increments per second
    ccp1con = 0b00000100;   // CCP capture mode ON, every falling edge of RB3 we capture the contents of tmr1 into CCPR1
}

// calculate our speed and write it to the "speed" variable
// "forceupdate" means ignore hysteresis and return raw speed
void calculate_speed(bit forceupdate)
{
    unsigned int twopulse_delta, lastpulse_delta, temp_lastpulses[2];

    static unsigned char lastupdatedtime = 0;
    static unsigned int rollingsum[6] = {0, 0, 0, 0, 0, 0}; // a rolling sum of the last 2, 4, 8, 16, 32, and 64 pulseswidths (estimated)
    static unsigned char lastpulsecount = 0;        // the last known pulsecount, based on least significant 8 bits
                                                    // (only recalculate speed if we have another pulse)

    int newspeed;   // signed int

    // if we don't have two valid pulses, we're done. Speed is zero.
    if (lastpulse_stale1 || lastpulse_stale0)
    {
        unsigned char i;
        for (i = 0; i < 6; i++)
            rollingsum[i] = 0;  // clear all rolling sums
        speed = 0;
        return;
    }

    SampleNow:
    // stop interrupts for a sec... we don't want the clocks messed with
    gie = 0;

    // This time, we cannot stop captures, so we should check consistency.
    now.mid8 = tmr1h;
    now.high8 = tmr1_upper;
    now.low8 = tmr1l;

    // if mid8 has changed, then low8 overflowed and high8 is also suspect.
    if (now.mid8 != tmr1h)
    {
        if (tmr1if)
        {
            // one in a million chance... tmr1 overflowed! oh noes!
            // tmr1_upper hasn't had a chance to update. We need to enable interrupts, go back, and try again.
            gie = 1;
            goto SampleNow;
        }
        // if we copy again, it should be fast enough to not overflow this time.
        now.mid8 = tmr1h;
        now.high8 = tmr1_upper;
        now.low8 = tmr1l;
    }

    temp_lastpulses[1] = make16(lastpulses[1]);
    temp_lastpulses[0] = make16(lastpulses[0]);
    // start the interrupts again
    gie = 1;

    // now.low16 = make16(now);
    lastpulse_delta = make16(now);

    // calculate difference between last 2 pulses' clock
    twopulse_delta = temp_lastpulses[0] - temp_lastpulses[1];
    // calculate difference between last pulse and now
    // lastpulse_delta = now.low16 - temp_lastpulses[0];
    lastpulse_delta -= temp_lastpulses[0];

    if (lastpulsecount != (unsigned char)pulsecount)    // NOTE: only update rolling average of times between two pulses
    {
        // we have a new pulse so we can update antijitter
        lastpulsecount = (unsigned char)pulsecount;
        // rollingsum[n] = ((2^n - 1) * rollingsum[n] / (2^n)) + (twopulse_delta)
        unsigned char i;
        for (i = 0; i < 6; i++)
        {
            rollingsum[i] -= (rollingsum[i] >> (i+1));
            rollingsum[i] += twopulse_delta;
        }
    }

    // whichever of those is greater, call that "period"
    // speed is calibfactor / period
    if (twopulse_delta > lastpulse_delta)
        newspeed = calib_factor / twopulse_delta;
    else if (lastpulse_delta == 0) // if (lastpulse_delta==0) && (twopulse_delta==0) catch the edge case so we don't divide by zero
        newspeed = 0;
    else    // in this case, our speed is lower because we haven't got a pulse yet as expected...
        newspeed = calib_factor / lastpulse_delta;

    // n = calib_factor / (twopulse_delta * 10)

    unsigned char n = (unsigned int)newspeed / 10;

    if (n)  // if going faster than 10kph, use an appropriate rolling average
    {
        unsigned char i = 0;    // "i" will tell us how many speed estimates to blend (2 for 10kph, 4 for 20, 8 for 40, and so on)
        while (n)
        {
            n = n >> 1;
            i++;
        }
        unsigned int sumpulse = (rollingsum[i-1] >> i);
        if (sumpulse)   // if (rollingsum[i-1] >> i) gets glitched to be zero, we'd be in serious trouble
            newspeed = calib_factor / sumpulse;
    }

    int t = (int)newspeed - speed;
    // we don't want to update the display if it hasn't changed much and it was already updated recently (within the last 0.25sec)
    if ((2 > t) && (t > -2) && (lastupdatedtime == (tmr1h & 0x80)) && !forceupdate)
        return;
    else
    {
        lastupdatedtime = tmr1h & 0x80;
        speed = (unsigned int)newspeed;
    }
}

// handles speed interrupt
void interrupt_speed(void)
{
    // tmr1 overflow, increment tmr1 "most significant bits" global var (since we can't set the prescaler high enough)
    // THIS MUST COME BEFORE CCP HANDLER
    if (tmr1if)
    {
        tmr1_upper++;
        // expire stale pulses
        if (tmr1_upper - lastpulses[1].high8 > PULSE_LIFETIME)
            lastpulse_stale1 = 1;
        if (tmr1_upper - lastpulses[0].high8 > PULSE_LIFETIME / 2)
            lastpulse_stale0 = 1;

        tmr1if = 0;
    }

    // tmr1 could roll over here! tmr1_upper would be inconsistent.

    // CCP trigger, happens when we get a speedometer pulse.
    if (ccp1if)
    {
        now.high8 = tmr1_upper;

        // Normally, stopping captures would be bad, but since we just triggered,
        // any captures that happen now are bounces.
        ccp1con = 0b00000000;       // stop it from capturing while we're reading the value
        now.mid8 = ccpr1h;
        now.low8 = ccpr1l;
        ccp1con = 0b00000100;

        // If tmr1if is uncleared, it means tmr1_upper we got is inconsistent.
        // Don't clear the ccp1if flag, we'll catch it on the next pass.
        if(!tmr1if)
        {
            // if it's too close to the last pulse, ignore it, it's just a bouncy switch

            if ((make16i(now) - make16i(lastpulses[0])) > SPEEDO_BOUNCE_THRESHOLD) // 2ms => 250 ticks => 62.5 bigticks
            {
                /*
                distance_pulses++;
                if (distance_pulses >= pulses_in_100m)
                {
                    distance_pulses = 0;
                    distance++;
                    if (distance > 9999)
                    {
                        distance = 0;
                    }
                    flag_distance = 1; // write to eprom

                    incr_distance();
                }
                */
                pulsecount++;
                lastpulses[1] = lastpulses[0];
                lastpulse_stale1 = lastpulse_stale0;
                lastpulses[0] = now;
                lastpulse_stale0 = 0;
            }
            ccp1if = 0;
        }   // else: don't worry about "now", as soon as we return from interrupt we'll hop back in again.
    }
}