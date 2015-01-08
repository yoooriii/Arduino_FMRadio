Arduino_FMRadio
===============

Simple FM radio made on Arduino + RRD-102 V2 (RDA5807M)
In this project I am playing with arduino and RDA5807M
I am planning to add RTC and improve RDS
Components:
FM Stereo Module Radio Module RDA5807M RRD-102 V2.0
Arduino Nano
Display 16x2 compatible with LiquidCrystal library
also I am going to make a 2nd version on a LED 7 segm x 4 digits display (no RDS in this case)
And a pair of computer speakers or headphones of course.


class RADIO_RDA5807M mostly based on Matthias Hertel, http://www.mathertel.de code
here I merged original 'radio' and 'RDA5807M' classes and added a bit of my logic
but generally I changed nothing. So if you encounter a bug take a look at the original
code chances are it is already fixed.
https://github.com/mathertel/Radio


real time clock
https://github.com/adafruit/RTClib
https://learn.adafruit.com/ds1307-real-time-clock-breakout-board-kit/understanding-the-code
http://www.bristolwatch.com/arduino/arduino_ds1307.htm


useful links including RDS:
http://en.wikipedia.org/wiki/Radio_Data_System
http://www.rds.org.uk/2010/Overview.htm
https://github.com/bastibl/gr-rds
https://github.com/gnuradio/gnuradio

RDS mostly works but I dont like the way it works. It is a feature of low priority and
I am not going to work on it now.

//TODO:
store found radiostations in flash
store volume and currently selected radio in flash
use LED 4 7 segm x 4 digits display
make menu and make buttons work with interruptions properly
RotationEncoder did not work well, figure out how to improve it (or get rid of it?)
