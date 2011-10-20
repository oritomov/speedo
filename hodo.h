#ifndef hodo_h
#define hodo_h

#include "common.h"

#define LPG_INPUT                           porta.4

void init_hodo(void);
void read_trip(void);
void incr_distance(void);
void write_distance(void);
void write_trip(unsigned int trip);
void interrupt_lpg(void);
uns24 distance(void);
unsigned int trip(void);
void clear_trip(void);
bit flag_lpg_reed(void);
bit flag_lpg_mode(void);
void flag_lpg_mode(bit value);

#endif hodo_h
