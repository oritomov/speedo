#ifndef button_h
#define button_h

#ifndef BUTTON_PIN
#define BUTTON_PIN    4
#endif //BUTTON_PIN

#ifndef BUTTON_DELAY
#define BUTTON_DELAY  5000
#endif //BUTTON_DELAY

#ifndef BUTTON_HELD
#define BUTTON_HELD   500
#endif //BUTTON_HELD

typedef enum {
  BUTTON_STATUS_NONE,
  BUTTON_STATUS_PRESSED,
  BUTTON_STATUS_HELD, 
  BUTTON_STATUS_TIMEOUT
} button_status;

class Button {
    unsigned int held_count; // how many tmr0 ticks the button has been held
    unsigned int rlsd_count; // how many tmr0 ticks the button has been released
    
  public:
#ifdef VOLATILE
    volatile 
#endif //VOLATILE
    button_status status;

    void init();
    void reset(void);
    void interrupt(void);
} button;

#endif //button_h

