This repo contains a simple system for driving a NeoPixel LED strip.

It consists of two programs, PixelAnimator, a C# command-line utility, and Player, an Arduino sketch.

PixelAnimator converts an image file into an uncompressed data file containing RGB values.

Player reads the RGB data files from an SD card and displays them line-by-line on a NeoPixel LED strip using the FastLED library.
It also contains a few simple routines for generating patterns algorithmicly. I don't currently use them because it is easier to
update the display by editing files on the SD card than reprogramming the Arduino.
