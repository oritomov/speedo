#ifndef button_h
#define button_h

typedef enum {
  BUTTON_STATUS_NONE,
  BUTTON_STATUS_PRESSED,
  BUTTON_STATUS_HELD, 
  BUTTON_STATUS_TIMEOUT
} button_status;

class Button {
    uint16_t held_count; // how many tmr0 ticks the button has been held
    uint16_t rlsd_count; // how many tmr0 ticks the button has been released 

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

