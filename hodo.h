#include <system.h>

#ifdef _LPG
#define LPG_INPUT                           porta.4
#endif //_LPG

void init_hodo(void);
void read_trip(void);
void incr_distance(void);
void write_distance(void);
