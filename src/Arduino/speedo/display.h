#ifndef display_h
#define display_h

#include <U8g2lib.h>

#define I2C_ADDR 0x70

#define BIG_DISPLAY 2
#ifndef SINGLE_DISPLAY
#define SMALL_DISPLAY 1
#endif //SINGLE_DISPLAY

class U8G2_SSD1306_128X_I2C : public U8G2 {
  public: U8G2_SSD1306_128X_I2C(uint8_t resolution, const u8g2_cb_t *rotation = U8G2_R0, uint8_t reset = U8X8_PIN_NONE, uint8_t clock = U8X8_PIN_NONE, uint8_t data = U8X8_PIN_NONE) : U8G2() {
    if (resolution == BIG_DISPLAY) {
      u8g2_Setup_ssd1306_i2c_128x64_noname_1(&u8g2, rotation, u8x8_byte_arduino_hw_i2c, u8x8_gpio_and_delay_arduino);
    } else {
      u8g2_Setup_ssd1306_i2c_128x32_univision_1(&u8g2, rotation, u8x8_byte_arduino_hw_i2c, u8x8_gpio_and_delay_arduino);
    }
    u8x8_SetPin_HW_I2C(getU8x8(), reset, clock, data);
  }
};

class AbstractDisplay {
  private:
    U8G2_SSD1306_128X_I2C u8g2;
    uint8_t number;

    void on(void);

  protected:
    AbstractDisplay(uint8_t number);
    void display(u8g2_uint_t x, u8g2_uint_t y, const uint8_t *font, const char *str);
#ifdef SINGLE_DISPLAY
    void two_lines(u8g2_uint_t x1, u8g2_uint_t y1, const uint8_t *font1, const char *str1,
                   u8g2_uint_t x2, u8g2_uint_t y2, const uint8_t *font2, const char *str2);
#endif //SINGLE_DISPLAY

  public:
    void init(void);
};

class BigDisplay: public AbstractDisplay {

  public:
    BigDisplay(void);
    void speed(uint8_t speed);
    void mode(String text);
#ifdef SINGLE_DISPLAY
    void mode(String text, int num);
#endif //SINGLE_DISPLAY
} bigDisplay;

#ifndef SINGLE_DISPLAY
class SmallDisplay: public AbstractDisplay {
  
  public:
    SmallDisplay(void);
    void trip(uint16_t trip);
    void num(int num);
} smallDisplay;
#endif //SINGLE_DISPLAY

#endif //display_h
