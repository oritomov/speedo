#ifndef button_h
#define button_h

class Button {
    unsigned int heldcount; // how many tmr0 ticks the button has been held
    
  public:
    boolean pressed;  // used to pass the message that a button press was registered
    boolean held;     // ... or a button was held.  Clear manually before and after use.

    void init();
    void reset(void);
    void interrupt(void);
} button;

#endif //button_h

