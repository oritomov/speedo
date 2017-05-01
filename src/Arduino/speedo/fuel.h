#ifndef fuel_h
#define fuel_h

class Fuel {
    int lpg_pin;

  public:
    byte flag_lpg_reed;          // use for lpg reed signal.
    byte flag_lpg_mode;          // use for unleaded or lpg mode.

    void init(int lpg_pin);
    void interrupt(void);
} fuel;

#endif //fuel_h

