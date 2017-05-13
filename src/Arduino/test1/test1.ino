#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2C_ADDR 0x3C

#define speedometer_pin 2

void setup() {
  Adafruit_SSD1306 display;
  
  display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR);  // initialize with the I2C addr (for the 128x64)
  // init done

  // Clear the buffer.
  display.clearDisplay();
  display.display();
  
  pinMode(speedometer_pin, OUTPUT);
}

void loop() {
  digitalWrite(speedometer_pin, HIGH);     // turn on the LED:
  delay(4);
  digitalWrite(speedometer_pin, LOW);     // turn off the LED:
  delay(4);
}
