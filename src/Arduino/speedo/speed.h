#ifndef speed_h
#define speed_h

class Speed {
  uint16_t calib_factor;    // calibration factor used to calculate speed

  uint16_t lastupdatedtime;
  uint16_t rollingsum[6];   // a rolling sum of the last 2, 4, 8, 16, 32, and 64 pulseswidths (estimated)
  uint16_t lastpulsecount;  // the last known pulsecount, based on least significant 8 bits
                            // (only recalculate speed if we have another pulse)
  public:
    uint8_t speed;          // our current speed, calculated in main loop (uint8_t only goes to 255)

    void init(void);
    void calculate(boolean forceupdate);
} speed;

#endif //speed_h
