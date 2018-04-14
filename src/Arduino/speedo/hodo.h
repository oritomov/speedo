#ifndef hodo_h
#define hodo_h

class Hodo {
    uint16_t pulses_in_100m;
    uint8_t distance_pulses;
    uint16_t trip_pulses;
    // flags determine storing to eprom
    boolean flag_trip:1;
    boolean flag_trip_half:1;
    flags flag_distance;
    
  public:
    uint24_t distance;
    uint16_t trip;

    void init();
    void reset(void);
    void read_trip(boolean flag_lpg_mode);
    void incr_distance(void);
    void write_distance(void);
} hodo;

#endif //hodo_h

