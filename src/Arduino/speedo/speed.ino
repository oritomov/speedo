#include <EEPROM.h>
#include "speed.h"

// eeprom
#define eeprom_calib_factor_kph_lo  0
#define eeprom_calib_factor_kph_hi  1

unsigned int calib_factor;      // calibration factor used to calculate speed
unsigned int speed;          // our current speed, calculated in main loop (unsigned char only goes to 255)
unsigned int pulsecount;      // updated on each pulse.  Used for drag timing.
unsigned long lastpulses[2];        // the last two pulses received.  index 0 is last, index 1 is the one before that.
boolean lastpulse_stale0;
boolean lastpulse_stale1;        // if a pulse happened too long ago, it's stale and we can't do anything with it

void init_speed(void)
{
  lastpulse_stale0 = false; // both pulses are invalid on reset; we need to get TWO pulses in a reasonable time
  lastpulse_stale1 = false; // before doing any calculating.
  
  // calib_factor
  {
    byte hi, lo;
    hi = EEPROM.read(eeprom_calib_factor_kph_hi);
    lo = EEPROM.read(eeprom_calib_factor_kph_lo);
    calib_factor = word(hi, lo);
    Serial.print("calib_factor = ");
    Serial.println(calib_factor);
//    if (calib_factor == 0xffff)
//      calib_factor = DEFAULT_CALIB_FACTOR;
//    byte hi = highByte(calib_factor);
//    byte lo = lowByte(calib_factor);
//    EEPROM.write(eeprom_calib_factor_kph_hi, hi);
//    EEPROM.write(eeprom_calib_factor_kph_lo, lo);
  }

  pinMode(speedometer_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(speedometer_pin), interrupt_speed, FALLING);
}

// happens when we get a speedometer pulse.
void interrupt_speed(void)
{
  unsigned long now = micros() >> 7;//millis();
    
  // if it's too close to the last pulse, ignore it, it's just a bouncy switch
  if ((now - lastpulses[0]) > SPEEDO_BOUNCE_THRESHOLD)  // 2ms
  {
    incr_distance();
    pulsecount++;
    lastpulses[1] = lastpulses[0];
    lastpulse_stale1 = lastpulse_stale0;
    lastpulses[0] = now;
    lastpulse_stale0 = true;
  }
}

// calculate our speed and write it to the "speed" variable
void calculate_speed()
{
  unsigned int twopulse_delta, lastpulse_delta, temp_lastpulses[2];

  static bool lastupdatedtime = 0;
  static unsigned int rollingsum[6] = {0, 0, 0, 0, 0, 0}; // a rolling sum of the last 2, 4, 8, 16, 32, and 64 pulseswidths (estimated)
  static unsigned char lastpulsecount = 0;    // the last known pulsecount, based on least significant 8 bits
                          // (only recalculate speed if we have another pulse)

  int newspeed; // signed int

  // if we don't have two valid pulses, we're done. Speed is zero.
  if (!lastpulse_stale1 || !lastpulse_stale0)
  {
    unsigned char i;
    for (i = 0; i < 6; i++)
      rollingsum[i] = 0;  // clear all rolling sums
    speed = 0;
    return;
  }
  
  // stop interrupt for a sec... we don't want the clocks messed with
  detachInterrupt(digitalPinToInterrupt(speedometer_pin));

  temp_lastpulses[1] = (int)lastpulses[1];
  temp_lastpulses[0] = (int)lastpulses[0];
    
  // start the interrupt again
  attachInterrupt(digitalPinToInterrupt(speedometer_pin), interrupt_speed, FALLING);

  unsigned long now = micros() >> 7;//millis();
  
  // now.low16 = make16(now);
  lastpulse_delta = (int)now;
  
  // calculate difference between last 2 pulses' clock
  twopulse_delta = temp_lastpulses[0] - temp_lastpulses[1];
  // calculate difference between last pulse and now
  // lastpulse_delta = now.low16 - temp_lastpulses[0];
  lastpulse_delta -= temp_lastpulses[0];

  if (lastpulsecount != (unsigned char)pulsecount)  // NOTE: only update rolling average of times between two pulses
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
  else if (lastpulse_delta == 0)  // if (lastpulse_delta==0) && (twopulse_delta==0) catch the edge case so we don't divide by zero
    newspeed = 0;
  else  // in this case, our speed is lower because we haven't got a pulse yet as expected...
    newspeed = calib_factor / lastpulse_delta;

  // n = calib_factor / (twopulse_delta * 10)

  int diff = 2;
  
  unsigned char n = (unsigned int)newspeed / 10;

  if (n != 0)  // if going faster than 10kph, use an appropriate rolling average
  {
    diff = 1;
    unsigned char i = 0;  // "i" will tell us how many speed estimates to blend (2 for 10kph, 4 for 20, 8 for 40, and so on)
    while (n != 0)
    {
      n = n >> 1;
      i++;
      diff = diff << 1;
    }
    unsigned int sumpulse = (rollingsum[i-1] >> i);
    if (sumpulse != 0) // if (rollingsum[i-1] >> i) gets glitched to be zero, we'd be in serious trouble
      newspeed = calib_factor / sumpulse;
  }

  int t = (int)newspeed - speed;
  
  // we don't want to update the display if it hasn't changed much and it was already updated recently (within the last 0.25sec)
  if ((diff > t) && (t > -diff) && (lastupdatedtime == (bool)(now & 0x2000)))
    return;
  else
  {
    lastupdatedtime = now & 0x2000;
    speed = (unsigned int)newspeed;
  }
}
