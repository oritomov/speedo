#ifndef hodo_h
#define hodo_h

typedef struct _uns24 {
  byte low8;
  byte mid8;
  byte high8;
} uns24;

typedef struct _flag_distance {
  boolean lo : 1;
  boolean mi : 1;
  boolean hi : 1;
  boolean half : 1;
} flags;

class Hodo {
    unsigned int pulses_in_100m;
    byte distance_pulses;
    unsigned int trip_pulses;
    // flags determine storing to eprom
    boolean flag_trip:1;
    boolean flag_trip_half:1;
    flags flag_distance;
    
  public:
    uns24 distance;
    unsigned int trip;

    void init();
    void reset(void);
    void read_trip(boolean flag_lpg_mode);
    void incr_distance(void);
    void write_distance(void);
} hodo;

#endif //hodo_h

