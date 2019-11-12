/*
Name:		Player.ino
Created:	12/3/2016 5:00:41 PM
Author:	david
*/


#include <FastLED.h>

//undef DEBUG
#define DEBUG

#define MAX_PIXELS 30

#ifdef DEBUG
#define log(s) Serial.write(s);
#else
#define log(s) // s
#endif // DEBUG

#include "LineSpin.h"
//#include "RedGreen.h"
//#include "YellowGo.h"
//#include "PurpleGo.h"
//#include "Noise.h"
//#include "Xmas.h"
//#include "rdeer.h"

struct {
	uint16_t mapWidth; // Width in pixels of the current map
	uint8_t frameTime; // Time in mS that each frame should appear
	int curRow;
	CRGB pxBuf[MAX_PIXELS]; // pixel data buffer 
} pixelMap;

void setup() {
	Serial.begin(119200);

	log("starting setup\n");

	pixelMap.curRow = 0;
	pixelMap.frameTime = FRAME_TIME;
	pixelMap.mapWidth = MAX_PIXELS;

	FastLED.addLeds<NEOPIXEL, 6>(pixelMap.pxBuf, MAX_PIXELS);
}

void loop() {
	pixelMap.curRow = 0;
	
	log("starting loop\n");

	FastLED.setMaxRefreshRate(1000 / pixelMap.frameTime);
	FastLED.setBrightness(30);
	
	// read and display each line
	while (pixelMap.curRow<31) {
		memcpy(pixelMap.pxBuf, testPat[pixelMap.curRow++], MAX_PIXELS * 3);
		FastLED.show();
	}
	log("completed play\n");
}
