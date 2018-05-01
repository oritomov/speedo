#ifndef hall_h
#define hall_h

class Hall {
  uint24_t lastpulses[2];       // the last two pulses received.  index 0 is last, index 1 is the one before that.
#ifdef VOLATILE
  volatile 
#endif //VOLATILE
  boolean lastpulse_stale0:1;
#ifdef VOLATILE
  volatile 
#endif //VOLATILE
  boolean lastpulse_stale1:1;   // if a pulse happened too long ago, it's stale and we can't do anything with it
  uint8_t tmr1_upper;           // since 0.5 seconds isn't long enough, we increment this with tmr1 overflows
                                //   and use it for top byte of lastpulses.
  public:
#ifdef VOLATILE
    volatile 
#endif //VOLATILE
    uint16_t pulsecount;      // updated on each pulse.  Used for drag timing.

    void init();
    boolean stale(void);
    void get_lastpulses(uint16_t *);
    void timer_interrupt(void);
    void hall_interrupt(void);
} hall;

#endif //hall_h
