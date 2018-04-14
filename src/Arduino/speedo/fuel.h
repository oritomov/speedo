#ifndef fuel_h
#define fuel_h

class Fuel {

  public:
    boolean flag_lpg_reed:1;          // use for lpg reed signal.

    void init();
    void interrupt(void);
} fuel;

#endif //fuel_h

