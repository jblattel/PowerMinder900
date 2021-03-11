# PowerMinder - Tesla Powerwall:
Currently there is no easy way for household members to know what the current state of power is at the house when using Tesla’s Powerwalls. (e.g. on battery due to a power outage, or on battery because of the time of day). Unless, you have installed the Tesla application and happen to have your phone with you.  Therefore, there are times when you would want to make the household aware of these potential situations so they can make informed power usage decisions.

## Solution:
Audio and Visual queues are ubiquitous. We find them when we are waiting for trains. We find them when we are waiting for the bus. We even see or hear them when we are cooking in the kitchen. The best way to signal power users is by providing a simple easy to understand mechanism.


## Requirements: 
We need a system that is flexible in many respects:
1) low power footprint
2) capable of being powered by battery only ( Lithium polymer )
3) doesn’t need to be connected to the internet (Tesla’s gateway runs on a local network and public Internet is not required
4) utilizes a wireless network
5) small enough so that it can be moved to strategic locations around the house easily

## Parts List (For this POC)
1) Argon from Particle (https://www.adafruit.com/product/3997)
2) OLED from Adafruit (https://www.adafruit.com/product/2900)
3) Ifrared Motion Detector (https://www.adafruit.com/product/4667)
4) Feather Doubler (https://www.adafruit.com/product/2890)
5) Some LED's I used these (https://www.adafruit.com/product/4204)
6) 900Mhz RFM Radio Tansiever (https://learn.adafruit.com/adafruit-feather-m0-radio-with-rfm69-packet-radio/overview)

Enclosure as required and wirring. Wiring diagram (TO BE ADDED)
A1 BLUE
A3 RED
A2 GREEN
D3 MOTION INPUT
OLED is I2C

## Proposal:
Particle I/O makes a very attractive and relatively inexpensive way to solve this problem utilizing their mesh networking systems. There are two major components to the system. The Argon which will serve as both a gateway ( has IP connectivity ) and a Mesh end point since we want to be able to use this device as an indicator as well. The Xenon which is a low cost mesh endpoint. This device will receive updates from the mesh network and set its indicator as appropriate. You can currently buy the development system directly from Particle here: https://store.particle.io/collections/wifi . 
Unfortunately, while this all sounds great Particle has subsequently deprecated their Mesh network solutions (https://blog.particle.io/mesh-deprecation/)
Which, has now forced a new radio solution the RFM69 - which is one of many different radio solutions out there. 


## The Journey:

Issues with Particle RFM69 driver - geared to Boron not necessaily other devices needed to change SPI initialization in the .cpp file. Also there are issues with the radio in conjunction with the OLED Feather. You need to adjust which buttons are implemented - like the Ethernet Featherwing the RFM69 won't work if using the A and B buttons.

While the Tesla Gateway provides lots of good information off the bat - newer iterations of the software require authentication and session managment - which I didn't want to make part of the microcontroller. Therefore I've moved off that work to a Flask Python Web app which serves as the intermediate between solutions. This also makes it easier to change paramters like heavy load paramaters so you don't need to reflash the hardware. The Python code can be found here: (XXXXX)

Now that we were getting a valid response back from the Telsa Gateway it is simply a matter of publishing the results up to the mesh network and taking action on them. 

LED STATUS:
    Breathing LED "ON BATTERY"
    Steady State LED "ON BATTERY - NO GRID" (not implemented)
    No LED "OFF PEAK"
OLED SHOWS:
    Battery Status, Percentage of Battery and Grid Status if Down.

## PowerMinder Client vs Gateway

NEED TO ADD ADDITIONAL INFORMATION HERE ON SPECIFICS AROUDN DATA SENT TO THE PACKET RADIO


## Information on 3D printed enclosure

NEED LINK TO FILES AND DETAILS