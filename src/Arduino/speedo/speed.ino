/*
Speed calculation subroutine.
Thanks to Jeff Hiner
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

#include "speed.h"

// happens when we get a speedometer pulse.
void interrupt_speed(void) {
  speed.interrupt();
}

void Speed::init()
{
  lastpulse_stale0 = false; // both pulses are invalid on reset; we need to get TWO pulses in a reasonable time
  lastpulse_stale1 = false; // before doing any calculating.
  
  calib_factor = eeprom_calib_factor();

  pinMode(SPEEDOMETER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SPEEDOMETER_PIN), interrupt_speed, FALLING);
}

// happens when we get a speedometer pulse.
void Speed::interrupt(void)
{
  unsigned int now = micros() >> 7;//millis();
    
  // if it's too close to the last pulse, ignore it, it's just a bouncy switch
  if ((now - lastpulses[0]) > SPEEDO_BOUNCE_THRESHOLD)  // 2ms
  {
    hodo.incr_distance();
    pulsecount++;
    lastpulses[1] = lastpulses[0];
    lastpulse_stale1 = lastpulse_stale0;
    lastpulses[0] = now;
    lastpulse_stale0 = true;
  }
}

// calculate our speed and write it to the "speed" variable
void Speed::calculate()
{
  unsigned int twopulse_delta, lastpulse_delta, temp_lastpulses[2];

  static bool lastupdatedtime = 0;
  static unsigned int rollingsum[6] = {0, 0, 0, 0, 0, 0}; // a rolling sum of the last 2, 4, 8, 16, 32, and 64 pulseswidths (estimated)
  static byte lastpulsecount = 0;    // the last known pulsecount, based on least significant 8 bits
                          // (only recalculate speed if we have another pulse)

  int newspeed; // signed int

  // if we don't have two valid pulses, we're done. Speed is zero.
  if (!lastpulse_stale1 || !lastpulse_stale0)
  {
    byte i;
    for (i = 0; i < 6; i++)
      rollingsum[i] = 0;  // clear all rolling sums
    speed = 0;
    return;
  }
  
  // stop interrupt for a sec... we don't want the clocks messed with
  detachInterrupt(digitalPinToInterrupt(SPEEDOMETER_PIN));

  //Serial.println(lastpulses[0] - lastpulses[1]);
  temp_lastpulses[1] = lastpulses[1];
  temp_lastpulses[0] = lastpulses[0];
    
  // start the interrupt again
  attachInterrupt(digitalPinToInterrupt(SPEEDOMETER_PIN), interrupt_speed, FALLING);

  unsigned int now = micros() >> 7;//millis();
  
  // now.low16 = make16(now);
  lastpulse_delta = now;
  
  // calculate difference between last 2 pulses' clock
  twopulse_delta = temp_lastpulses[0] - temp_lastpulses[1];
  // calculate difference between last pulse and now
  // lastpulse_delta = now.low16 - temp_lastpulses[0];
  lastpulse_delta -= temp_lastpulses[0];

  if (lastpulsecount != pulsecount)  // NOTE: only update rolling average of times between two pulses
  {
    // we have a new pulse so we can update antijitter
    lastpulsecount = pulsecount;
    // rollingsum[n] = ((2^n - 1) * rollingsum[n] / (2^n)) + (twopulse_delta)
    byte i;
    for (i = 0; i < 6; i++)
    {
      rollingsum[i] -= (rollingsum[i] >> (i+1));
      rollingsum[i] += twopulse_delta;
//      Serial.print(rollingsum[i]);
//      Serial.print(", ");
    }
  }

  // whichever of those is greater, call that "period"
  // speed is calibfactor / period
  if (twopulse_delta > lastpulse_delta)
    newspeed = calib_factor / twopulse_delta;
  else if (lastpulse_delta == 0)  // if (lastpulse_delta==0) && (twopulse_delta==0) catch the edge case so we don't divide by zero
    newspeed = 0;
  else  // in this case, our speed is lower because we haven't got a pulse yet as expected...
    newspeed = calib_factor / lastpulse_delta;

  // n = calib_factor / (twopulse_delta * 10)

  int diff = 2;
  
  byte n = newspeed / 10;

  if (n != 0)  // if going faster than 10kph, use an appropriate rolling average
  {
    diff = 1;
    byte i = 0;  // "i" will tell us how many speed estimates to blend (2 for 10kph, 4 for 20, 8 for 40, and so on)
    while (n != 0)
    {
      n = n >> 1;
      i++;
      diff = diff << 1;
    }
//    Serial.print(i);
//    Serial.print(", ");
    unsigned int sumpulse = (rollingsum[i-1] >> i);
//    Serial.print(sumpulse);
    if (sumpulse != 0) // if (rollingsum[i-1] >> i) gets glitched to be zero, we'd be in serious trouble
      newspeed = calib_factor / sumpulse;
  }

  int t = newspeed - speed;
  
//  Serial.print(", ");
//  Serial.print(diff);
//  Serial.print(", ");
//  Serial.print(lastupdatedtime);
//  Serial.print(", ");
//  Serial.print((bool)now & 0x2000);
//  Serial.print(", ");
//  Serial.print(diff > t);
//  Serial.print(", ");
//  Serial.print(t > -diff);
//  Serial.print(", ");
//  Serial.print(lastupdatedtime == (bool)(now & 0x2000));
//  Serial.print(", ");
////  Serial.println(now, HEX);
////  Serial.println(speed);
  
  // we don't want to update the display if it hasn't changed much and it was already updated recently (within the last 0.25sec)
  if ((diff > t) && (t > -diff) && (lastupdatedtime == (bool)(now & 0x2000)))
//  {
//    Serial.println(speed);
    return;
//  }
  else
  {
    lastupdatedtime = now & 0x2000;
    speed = newspeed;
//    Serial.println(speed);
  }
}
