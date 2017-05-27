#ifndef display_h
#define display_h

#include <U8g2lib.h>

#define I2C_ADDR 0x70

#define BIG_DISPLAY 1
#define SMALL_DISPLAY 2

class U8G2_SSD1306_128X_I2C : public U8G2 {
  public: U8G2_SSD1306_128X_I2C(byte resolution, const u8g2_cb_t *rotation = U8G2_R0, uint8_t reset = U8X8_PIN_NONE, uint8_t clock = U8X8_PIN_NONE, uint8_t data = U8X8_PIN_NONE) : U8G2() {
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
    byte number;

    void on(void);

  protected:
    AbstractDisplay(byte number);
    void display(u8g2_uint_t x, u8g2_uint_t y, const uint8_t *font, const char *str);

  public:
    void init(void);
};

class BigDisplay: public AbstractDisplay {

  public:
    BigDisplay(void);
    void speed(unsigned int speed);
    void mode(String text);
} bigDisplay;

class SmallDisplay: public AbstractDisplay {
  
  public:
    SmallDisplay(void);
    void trip(unsigned int trip);
    void num(int num);
} smallDisplay;

#endif //display_h
