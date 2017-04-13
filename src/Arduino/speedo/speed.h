#ifndef speed_h
#define speed_h

#define speedometer_pin 2

// how many overflows of tmr1 before a pulse is considered "stale" (invalid)
#define PULSE_LIFETIME      7
// 62 bigticks = 1.98ms
#define SPEEDO_BOUNCE_THRESHOLD  62

void init_speed(void);
void interrupt_speed(void);
void calculate_speed(boolean forceupdate);

extern unsigned int pulsecount;
extern unsigned int speed;

#endif speed_h

