# BlueFruit-LED-Controller
Arduino code for controlling an Adafruit Feather 32u4 and LED strip with the BlueFruit App in Controller Mode


This code was designed for use in conjunction with 2 Neopixel Sticks and mounted as shown:
https://imgur.com/gallery/zXU4OJ7

The code utilises the Controller module in the BlueFruit app in which the following modes and settings exist:
- The numbers 1, 2, 3, 4, toggle the Red, Green, Blue, and White LED's respectively in any mode (Rainbow will not show this change, but switching to a different mode will)
- The Left and Right arrows cycle between 4 different modes: Solid, Fade, Strobe, and Rainbow
- The Up and Down arrows control the Brightness and Fade/Strobe/Transition speed depending on the mode, with the Brightness and Speed settings persisting across different modes.

The code utilises a state machine approach due to input hanging encountered within the fading and rainbow loop.
