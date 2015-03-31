# Speedo
The micro controller is based on PIC16F648A and 7 segment led displays. Presents digital speedometer and odometer. 

## Introduction

The micro controller presents digital speedometer and odometer. 
It is implemented especially for Audi B3/B4 with bi-fuel - unleaded and LPG.


## Details

### Circuit

![](http://speedo.googlecode.com/files/speedo.png)

#### Modules

  * Pic - PIC16F648A
  * Decoder - SN74LS138N
  * Display - 7 segment led displays common cathode 

#### Inputs

  * Speedo Reed
  * LPG
  * Mode selector

### Main board

![](http://speedo.googlecode.com/files/top_copper.png)

## Releases

  * Alpha phase

  ![](http://www.bgphoto.net/photos/16099/o633995982031250000.JPG)
  
  * Beta phase

  ![](http://speedo.googlecode.com/files/DSC00362.JPG)
  
  * Gamma phase

  ![](http://speedo.googlecode.com/files/DSC00361.JPG)
  ![](http://speedo.googlecode.com/files/DSC00363.JPG)

## References

  * [DIY digital speedometer using PIC microcontroller by Jeff Hiner](http://www.randomwisdom.com/2007/10/digital-speedometer-using-pic-microcontroller/)
  * Microchip [Datasheet](http://www.microchip.com/wwwproducts/Devices.aspx?dDocName=en010212 PIC16F648A] [http://ww1.microchip.com/downloads/en/devicedoc/40044e.pdf)
  * 3-Line To 8-Line Decoders/Demultiplexers [SN74LS138N](http://www.ti.com/product/sn74ls138) [Datasheet](http://www.ti.com/lit/ds/symlink/sn74ls138.pdf)
  * [Orient Display 6 digit 7segment LCD](http://www.orientdisplay.com/standard_lcd/od-601.pdf)
  * [LCD Driving Circuitry: Static](http://www.good-lcd.com/upfile/product/200864124829157.pdf)
  * [AVR241: Direct driving of LCD display using general IO](http://www.atmel.com/dyn/resources/prod_documents/doc2569.pdf)
  * [Driving static LCD displays](http://www.ubasics.com/driving_static_lcds)
  * [Thermometer with an LCD display](http://mcs.uwsuper.edu/sb/Electronics/Termo/)
  * [Speedometer Signal Conditioner](http://forums.pelicanparts.com/uploads14/SpeedometerSignalConditionerCircuit1222356206.jpg)
