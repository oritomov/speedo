#ifndef display_h
#define display_h

#include <U8g2lib.h>

#define I2C_ADDR 0x3C

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

class Abstract_Display {
  protected:
    U8G2_SSD1306_128X_I2C u8g2;
    byte display;

    Abstract_Display(byte display);

  public:
    void init(void);
};

class Big_Display: public Abstract_Display {

  public:
    Big_Display(byte display);
    void speed(unsigned int speed);
    void mode(String text);
    void mode(String text, int p);
} display(BIG_DISPLAY), display2(SMALL_DISPLAY);

#endif //display_h
