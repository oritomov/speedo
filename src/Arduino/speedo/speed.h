#ifndef speed_h
#define speed_h

// (14" * 2.54cm * 0.01 m/cm + 2 * 195mm * 0.001 m/mm * 60%) * pi => 1.85m
// 100m / 1.85m * 8 pulses/turn = 54 turns/100m * 8 pulses/turn = 432 pulses / 100m
//#define DEFAULT_PULSES_IN_100M      432
// 3600 sec/h * 1000000 us/sec * 0.1 km / 128 = 2812500 us * 100m / h
// 2812500 / 432 pulses_in_100m = 6510
//#define DEFAULT_CALIB_FACTOR  6510

// 2ms
#define SPEEDO_BOUNCE_THRESHOLD  16

#define speedometer_pin 2

extern unsigned int speed;

void init_speed(void);
void interrupt_speed(void);
void calculate_speed(void);

#endif
