#ifndef hodo_h
#define hodo_h

#include "common.h"

#define LPG_INPUT							porta.5

void init_hodo(void);
void read_trip(void);
void incr_distance(void);
void write_distance(void);
void write_trip(unsigned int trip);
void interrupt_lpg(void);

extern uns24 distance;
extern unsigned int trip;
extern unsigned int trip_pulses;
extern bit flag_lpg_reed;
extern bit flag_lpg_mode;

#endif hodo_h
