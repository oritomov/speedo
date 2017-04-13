#include <EEPROM.h>
#include "speed.h"

// eeprom
#define eeprom_calib_factor_kph  0

unsigned int calib_factor;      // calibration factor used to calculate speed
unsigned int pulsecount;      // updated on each pulse.  Used for drag timing.
unsigned long lastpulses[2];        // the last two pulses received.  index 0 is last, index 1 is the one before that.
boolean lastpulse_stale0;
boolean lastpulse_stale1;        // if a pulse happened too long ago, it's stale and we can't do anything with it
unsigned long now;              // last sampled tmr1/tmr1_upper "now" time.

void init_speed(void)
{
  // calib_factor
  calib_factor = EEPROM.read(eeprom_calib_factor_kph);

  lastpulse_stale0 = 1; // both pulses are invalid on reset; we need to get TWO pulses in a reasonable time
  lastpulse_stale1 = 1; // before doing any calculating.

  now = 0;
  
  pinMode(speedometer_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(speedometer_pin), interrupt_speed, FALLING);
  
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

// Interrupt is called once a millisecond, 
SIGNAL(TIMER0_COMPA_vect) 
{
  now = millis();
  
//  if () // every 524.288 ms
//  {
  // expire stale pulses
  if (now - lastpulses[1] > PULSE_LIFETIME)
    lastpulse_stale1 = 1;
  if (now - lastpulses[0] > PULSE_LIFETIME / 2)
    lastpulse_stale0 = 1;
//  }
}

// happens when we get a speedometer pulse.
void interrupt_speed(void)
{
  now = millis();

  // if it's too close to the last pulse, ignore it, it's just a bouncy switch
  if ((now - lastpulses[0]) > SPEEDO_BOUNCE_THRESHOLD)  // 2ms => 250 ticks => 62.5 bigticks
  {
    incr_distance();

    pulsecount++;
    lastpulses[1] = lastpulses[0];
    lastpulse_stale1 = lastpulse_stale0;
    lastpulses[0] = now;
    lastpulse_stale0 = 0;
  }
}

// calculate our speed and write it to the "speed" variable
// "forceupdate" means ignore hysteresis and return raw speed
void calculate_speed(boolean forceupdate)
{
  unsigned int twopulse_delta, lastpulse_delta, temp_lastpulses[2];

  static unsigned int rollingsum[6] = {0, 0, 0, 0, 0, 0}; // a rolling sum of the last 2, 4, 8, 16, 32, and 64 pulseswidths (estimated)
  static unsigned char lastpulsecount = 0;    // the last known pulsecount, based on least significant 8 bits
                          // (only recalculate speed if we have another pulse)

  int newspeed; // signed int

  // if we don't have two valid pulses, we're done. Speed is zero.
  if (lastpulse_stale1 || lastpulse_stale0)
  {
    unsigned char i;
    for (i = 0; i < 6; i++)
      rollingsum[i] = 0;  // clear all rolling sums
    speed = 0;
    return;
  }

  // stop interrupts for a sec... we don't want the clocks messed with
  detachInterrupt(digitalPinToInterrupt(speedometer_pin));

  now = millis();

  temp_lastpulses[1] = (int)lastpulses[1];
  temp_lastpulses[0] = (int)lastpulses[0];
  // start the interrupts again
  attachInterrupt(digitalPinToInterrupt(speedometer_pin), interrupt_speed, FALLING);
  
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

  unsigned char n = (unsigned int)newspeed / 10;

  if (n)  // if going faster than 10kph, use an appropriate rolling average
  {
    unsigned char i = 0;  // "i" will tell us how many speed estimates to blend (2 for 10kph, 4 for 20, 8 for 40, and so on)
    while (n)
    {
      n = n >> 1;
      i++;
    }
    unsigned int sumpulse = (rollingsum[i-1] >> i);
    if (sumpulse) // if (rollingsum[i-1] >> i) gets glitched to be zero, we'd be in serious trouble
      newspeed = calib_factor / sumpulse;
  }

  int t = (int)newspeed - speed;
  // we don't want to update the display if it hasn't changed much and it was already updated recently (within the last 0.25sec)
  if ((2 > t) && (t > -2) && !forceupdate)
    return;
  else
  {
    speed = (unsigned int)newspeed;
  }
}

