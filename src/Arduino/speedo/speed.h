#ifndef speed_h
#define speed_h

// (14" * 2.54cm * 0.01 m/cm + 2 * 195mm * 0.001 m/mm * 60%) * pi => 1.85m
// 100m / 1.85m * 8 pulses/turn = 54 turns/100m * 8 pulses/turn = 432 pulses / 100m
//#define DEFAULT_PULSES_IN_100M      432
// 3600 sec/h * 1000000 us/sec * 0.1 km / 32 = 11250000 us * 100m / h
// 11250000 / 432 pulses_in_100m = 26041.66666666667
//#define DEFAULT_CALIB_FACTOR  26042

// 1 << 5 = 32
#define DIVIDER 5

// 2ms
#define SPEEDO_BOUNCE_THRESHOLD  16

class Speed {
    unsigned int calib_factor;    // calibration factor used to calculate speed
    unsigned int pulsecount;      // updated on each pulse.  Used for drag timing.
    unsigned int lastpulses[2];  // the last two pulses received.  index 0 is last, index 1 is the one before that.
    boolean lastpulse_stale0:1;
    boolean lastpulse_stale1:1;     // if a pulse happened too long ago, it's stale and we can't do anything with it

  public:
    unsigned int speed;           // our current speed, calculated in main loop (unsigned char only goes to 255)

    void init();
    void interrupt(void);
    void calculate(void);
} speed;

#endif //speed_h
