#ifndef fuel_h
#define fuel_h

class Fuel {

  public:
    boolean flag_lpg_reed;          // use for lpg reed signal.
    boolean flag_lpg_mode;          // use for unleaded or lpg mode.

    void init();
    void interrupt(void);
} fuel;

#endif //fuel_h

