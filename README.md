makeBeer
=======

## Making beer, nerd style
A home made **embedded** beer brewing temperature monitor  
and control system.

## Equipment
* Arduino pro micro 5v micro processor
* DS18B20 temperature probe
* A couple cool looking switches
    * One 3 way switch for temp adjustment up/down
    * (One 2 way switch for on/off pump, maybe)
* 20x4 LCD display
* A box to put the microprocessor inside of
* Optional:
    * ESP8266 Wi-Fi module for your thing to make a web app to control the module from your couch
    * Home made connectors so temperature probe and heater can easily be unplugged
* Mandatory:
    * Day dreams about nerdy programming, building and brewing

## WORK IN PROGRESS
Heres the plan:
### STEP 1
The code contained in offline.cpp will be copy/pasted into Arduino IDE,
then checked/compiled/flashed onto an Arduino pro micro.
The temperature probe and various hardware will be hooked up,
and the unit will be tested and code adjusted accordingly.

### STEP 2
A series of manual experiments of bringing various volumes of water
to specified temperatures will be performed. This will be used to tweak
and adjust the Kp, Ki, and Kd variables of the PID controller.

### STEP 3
The entire set up will be assembled and tested in an actual brewing setting once
everything is thoroughly tested and adjusted.

### STEP 4
The code contained in wifi.cpp will be flashed to an ESP8266 WiFi module,
once completed (it's still **very much** a work in progress).
The code in main.cpp will be flashed to an Arduino pro micro,
once completed (also very much a work in progress).
This would allow for remote monitoring and control of temperatures from,
say, the couch, during longer brewing sessions.

## DISCLAIMER
The entire project is very much in an incomplete state, any use is **entirely at your own peril**.
It is entirely for learning purposes, mostly to learn some C++.
Also, it's just a cool and fun project!
