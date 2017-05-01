#ifndef display_h
#define display_h

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2C_ADDR 0x3C

class Display {
  Adafruit_SSD1306 display;

  public:
    void init(void);
    void speed(unsigned int speed);
    void mode(int x, int y, int s, String text);
    void mode(int x, int y, int s, String text, int p);
} display;

#endif //display_h
