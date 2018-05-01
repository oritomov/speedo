# Speedo
Digital speedometer and odometer based on Arduino and OLED displays. 
It is implemented especially for Audi B3/B4 with bi-fuel - unleaded and LPG.

## Introduction

There is a UAF2115 in the spidometer. It uses speedo read (hall) sensor and shows the speed and mileage. 

![](https://github.com/oritomov/speedo/blob/master/git/arduino/uaf2115.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/uaf2115.png)

Shown speed is not accrued and mileage mechanism is made from plastic and after almost 30 years it is useless.

That is why I decided to build it digital.

10 years before! Or something :)

## Details

Initially I thought to gather all the electronics based on a PIC and 7 segments LEDs at the opposite side of the UAF2115 into the speedometer. But I never release it at that manner.

Recently I decided to build it by Arduino and OLED displays. But then I never find a way to gather all electronics into speedometer. And finally I made it with the main board outside.

I use the reset odometer button for mode selector. Short push is change of mode, and held of button for confirm. For example: push changes mode to "Reset mileage?" mode. If it follows push - then mode changes to "Set tires?" or if the button is held - reset indeed has happen. 

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
![](https://github.com/oritomov/speedo/blob/master/git/arduino/proto4.JPG)

  * pcb
  
![](https://github.com/oritomov/speedo/blob/master/git/arduino/pcb_top.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/pcb_bottom.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/pcb1.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/pcb2.JPG)

  * button
  
![](https://github.com/oritomov/speedo/blob/master/git/arduino/button1.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/button2.JPG)

  * beds for displays
  
![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_beds.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_in_beds.JPG)

  * frame for displays
  
![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_frame.JPG)


  * displays
  
![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_framed.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_mounted.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/l_cable.JPG)
![](https://github.com/oritomov/speedo/blob/master/git/arduino/display_test.JPG)


  * complete & working
  
![](https://github.com/oritomov/speedo/blob/master/git/arduino/ready_and_working.JPG)

## References

  * [DIY digital speedometer using PIC microcontroller by Jeff Hiner](http://www.randomwisdom.com/2007/10/digital-speedometer-using-pic-microcontroller/)
  * [first release based on pic & led](https://github.com/oritomov/speedo/blob/master/git/pic/README.md)
