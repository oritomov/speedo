# Speedo
Digital speedometer and odometer based on Arduino and OLED displays. 
It is implemented especially for Audi B3/B4 with bi-fuel - unleaded and LPG.

## Introduction

This is the original UAF2115 in the spidometer. It uses speedo read (hall) sensor and shows the speed and mileage. 

![](https://github.com/oritomov/speedo/blob/master/git/arduino/uaf2115.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/uaf2115.png)

Shown speed is not accurate and mileage mechanism is made from plastic and after almost 30 years it is useless.

That is why I decided to build a digital one.

10 years ago! Or something :)

There are two mileages for the different fuels - one for LPG and one for unleaded. It shows the current one.

I use the reset odometer button for mode selector. Short push is changing mode, and helding the button to confirm. For example: one push changes to "Reset mileage?" mode. If the button is held - reset indeed has happened or if it follows second push - then mode changes to "Set tires?" and etc. 

## Details

Initially I thought to gather all the electronics based on a PIC and 7 segments LEDs at the opposite side of the UAF2115 into the speedometer. But I never released in at that manner.

Recently I decided to build it with Arduino and OLED displays. But then I never find a way to gather all electronics into speedometer. And finally I made it with the main circuitboard outside.

### Circuit

![](https://github.com/oritomov/speedo/blob/master/git/arduino/scheme.png)

#### Modules

  * Arduino Nano
  * I2C Switch - TCA9543A
  * Display - two I2C OLEDs 

#### Inputs

  * Speedo read
  * LPG
  * Mode selector

### Main board

![](https://github.com/oritomov/speedo/blob/master/git/arduino/pcb.png)

## Release

  * prototype

![](https://github.com/oritomov/speedo/blob/master/git/arduino/proto1.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/proto2.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/proto3.JPG)


 * inside board. it sits at the opposite side of the UAF2115. There is a housing for the cable to the main board, a pin for LPG,  other pins for the displays, the button and also power, ground and speed signal from the UAF2115

![](https://github.com/oritomov/speedo/blob/master/git/arduino/proto4.JPG)


  * the main pcb

![](https://github.com/oritomov/speedo/blob/master/git/arduino/pcb_top.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/pcb_bottom.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/pcb1.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/pcb2.JPG)


  * button. I glued some plastic blocks to screw the button 

![](https://github.com/oritomov/speedo/blob/master/git/arduino/button1.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/button2.JPG)


  * beds for displays. I dig couple mm into front plate where displays should lays

![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_beds.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_in_beds.JPG)


  * frame for displays. And glue some extra ribs to support displays at the back.

![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_frame.JPG)


  * At this manner displays are fixed in place, but I can change them in case.

![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_framed.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_mounted.JPG)


  * In order display connectors not to touch anything inside speed point mechanism, I made them L shaped

![](https://github.com/oritomov/speedo/blob/master/git/arduino/l_cable.JPG)


  * almost assembled

![](https://github.com/oritomov/speedo/blob/master/git/arduino/assembly1.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/assembly2.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/assembly3.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_test.JPG)


  * and everything is complete & working

![](https://github.com/oritomov/speedo/blob/master/git/arduino/ready_and_working.JPG)

## References

  * [DIY digital speedometer using PIC microcontroller by Jeff Hiner](http://www.randomwisdom.com/2007/10/digital-speedometer-using-pic-microcontroller/)
  * [first release based on pic & led](https://github.com/oritomov/speedo/blob/master/git/pic/README.md)
