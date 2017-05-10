#ifndef display_h
#define display_h

#include <U8g2lib.h>

#define I2C_ADDR 0x3C

class Display {
  U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;

  public:
    Display(const u8g2_cb_t *rotation, uint8_t reset);
    void init(void);
    void speed(unsigned int speed);
    void mode(String text);
    void mode(String text, int p);
} display(U8G2_R0, U8X8_PIN_NONE);

#endif //display_h
