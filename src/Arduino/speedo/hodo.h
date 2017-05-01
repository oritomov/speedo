#ifndef hodo_h
#define hodo_h

typedef struct _uns24 {
  byte low8;
  byte mid8;
  byte high8;
} uns24;

class Hodo {
    unsigned int pulses_in_100m;
    byte distance_pulses;
    unsigned int trip_pulses;
    // flags determine storing to eprom
    byte flag_distance_lo;
    byte flag_distance_mi;
    byte flag_distance_hi;
    byte flag_distance_half;
    byte flag_trip;
    byte flag_trip_half;
    
  public:
    uns24 distance;
    unsigned int trip;

    void init();
    void reset(void);
    void read_trip(void);
    void incr_distance(void);
    void write_distance(void);
    void write_trip(unsigned int trip);
} hodo;

#endif //hodo_h

