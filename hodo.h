#ifndef hodo_h
#define hodo_h

#include "common.h"

#define LPG_INPUT                           porta.4

void init_hodo(void);
void read_trip(void);
void incr_distance(void);
void write_distance(void);
void interrupt_lpg(void);

#endif hodo_h
