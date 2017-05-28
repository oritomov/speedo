#ifndef button_h
#define button_h

class Button {
    unsigned int heldcount; // how many tmr0 ticks the button has been held
    
  public:
    boolean pressed:1;  // used to pass the message that a button press was registered
    boolean held:1;     // ... or a button was held.  Clear manually before and after use.

    void init();
    void reset(void);
    void interrupt(void);
} button;

#endif //button_h

