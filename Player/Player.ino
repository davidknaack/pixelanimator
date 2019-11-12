/*
 Name:		Player.ino
 Created:	12/3/2016 5:00:41 PM
 Author:	david
*/

#include <FastLED.h>
#include <SPI.h>
#include <SD.h>

//* SD card attached to SPI bus as follows :
//** MOSI - pin 11
//* * MISO - pin 12
//* * CLK - pin 13
// ** CS - pin 4

#define DEBUG
//#undef DEBUG

#define MAX_PIXELS 300
#define BRIGHT 8 // Analog 8 for brightness input

#ifdef DEBUG
#define log(s) Serial.print(s);
#define log2(s,t) Serial.print(s); Serial.print(t);
#define logn(s) Serial.print(s); Serial.print("\n");
#define log2n(s,t) Serial.print(s); Serial.print(t); Serial.print("\n");
#else
#define log(s) // s
#define log2(s,t) // s
#define logn(s) // n
#define log2n(s,t) // n
#endif // DEBUG

struct {
	File file; // File containing pixelmap data
	uint16_t mapWidth; // Width in pixels of the current map
	uint8_t frameTime; // Time in mS that each frame should appear
	uint8_t repeatCount; // Number of times to replay the file
	CRGB pxBuf[MAX_PIXELS]; // pixel data buffer 
} pixelMap;

void setup() {
	Serial.begin(119200);
	while (!Serial) {}

	FastLED.addLeds<NEOPIXEL, 6>(pixelMap.pxBuf, MAX_PIXELS);

	logn("SD init");
	if (!SD.begin(4)) {
		logn("SD init failed");
		return;
	}
}

// Open the specified pixelmap file and read the header
bool pxmOpen(const char* fName){
	if (!SD.exists(fName)) {
		log2n("cannot open pixelmap, not found: ", fName);
		return false;
	}

	if (pixelMap.file) {
		logn("closing previous file");
		pixelMap.file.close();
	}

	log2n("opening ", fName);
	pixelMap.file = SD.open(fName);
	if (!pixelMap.file) {
		log2n("error opening ", fName);
		return false;
	}

	if (-1 == pixelMap.file.read(&pixelMap.mapWidth, 2)) {
		logn("error reading mapWidth");
		return false;
	}

	if (-1 == pixelMap.file.read(&pixelMap.frameTime, 1)) {
		logn("error reading frame time");
		return false;
	}

	if (-1 == pixelMap.file.read(&pixelMap.repeatCount, 1)) {
		logn("error reading repeat count");
		return false;
	}

	return true;
}

// pxmNextPixel reads the next line from the open pixelmap file into the buffer
bool pxmNextLine() {
	log2n("reading next line, pixel width: ", pixelMap.mapWidth);

	uint16_t readSize = pixelMap.mapWidth * sizeof(CRGB);
	uint16_t skipSize = 0;

	if (pixelMap.mapWidth > MAX_PIXELS) {
		readSize = MAX_PIXELS * sizeof(CRGB);
		skipSize = (pixelMap.mapWidth - MAX_PIXELS) * sizeof(CRGB);
	}

	uint32_t startp = pixelMap.file.position();
	log2n("initial pos: ", pixelMap.file.position()); 
	log2n("to read: ", readSize); 
	int res = pixelMap.file.read(pixelMap.pxBuf, readSize);
	log2n("read:", res);

	log2n("new pos: ", pixelMap.file.position());
	log2n("target pos: ", startp + pixelMap.mapWidth * sizeof(CRGB));
	log2n("skip: ", skipSize);
	pixelMap.file.seek(pixelMap.file.position() + skipSize);

	log2n("end pos:", pixelMap.file.position());

	return res;
}

void fillBuffer(uint16_t ofs, CRGB color){
	uint16_t j = ofs;

	// Build the pixel buffer
	for (uint16_t i = 0; i < MAX_PIXELS - 1; i++)
	{
		if ((j % 4) == 0)
			pixelMap.pxBuf[i] = color;
		else
			pixelMap.pxBuf[i] = CRGB(0, 0, 0);

		j++;
		j = j % 4;
	}
}

void colorChase(CRGB color, uint8_t delayTime, uint8_t loops) {
	for (uint8_t i = 0; i < loops; i++)
	{
		fillBuffer(i % 4, color);
		FastLED.show();
		delay(delayTime);
	}
}

void RedToGreenFadeChase(uint8_t chaseDelay) {
	CRGB redDec = CRGB(4, 0, 0);
	CRGB grnInc = CRGB(0, 4, 0);

	CRGB cur = CRGB(0xff, 0, 0);

	for (uint8_t i = 0; i < 64; i++)
	{
		cur = cur - redDec;
		cur = cur + grnInc;
		colorChase(cur, chaseDelay, 4);
	}
}

void ToBlackFadeChase(CRGB color, uint8_t chaseDelay) {
	CRGB cur = color;
	CRGB dec = CRGB(8, 8, 8);

	for (uint8_t i = 0; i < 32; i++)
	{
		cur = cur - dec;
		colorChase(cur, chaseDelay, 4);
	}
}

void BluePulse() {
	uint8_t delayTime = 50;
	CRGB cur = CRGB(0, 0, 0xff);
	CRGB dec = CRGB(0, 0, 4);

	for (uint8_t j = 0; j < 10; j++)
	{


		for (uint8_t i = 0; i < 60; i++)
		{
			cur = cur - dec;

			fillBuffer(0, cur);
			FastLED.show();
			delay(delayTime);
		}

		for (uint8_t i = 0; i < 60; i++)
		{
			cur = cur + dec;

			fillBuffer(0, cur);
			FastLED.show();
			delay(delayTime);
		}
	}
}

// Do some scripted displays
void DoScriptedDisplay() {
	uint8_t chaseDelay = 75;

	colorChase(CRGB(0xff, 0, 0), chaseDelay, 40);
	RedToGreenFadeChase(chaseDelay);
	ToBlackFadeChase(CRGB(0, 0xff, 0), chaseDelay);
	delay(2000);
	BluePulse();
}

void loop() {
	uint8_t fIndex = 0;
	char fName[6];
	
	logn("starting file list");

	// Read numbered sequence of files 01.pxm, 02.pxm, etc.
	while (true) {
		// Generate name of next file
		sprintf(fName, "%02u.pxm", ++fIndex);

		// Does it exist?
		if (!SD.exists(fName)) {
			log2n("file not found: ", fName);
			break;
		}

		// open the file
		if (!pxmOpen(fName)) {
			logn("file open error, skipping");
			return;
		}
		log2n("file size: ", pixelMap.file.size());
		log2("frame time: ", pixelMap.frameTime); logn("mS");

		FastLED.setMaxRefreshRate(1000 / pixelMap.frameTime);
		log2n("playing file: ", fName);
		
		// Loop the specified number of times
		for (uint8_t i = 0; i < pixelMap.repeatCount; i++)
		{
			log2n("rep: ", i);
		
			// read and display each line
			while (pxmNextLine()) {
				uint8_t brightness = map(analogRead(BRIGHT), 0, 1023, 0, 255);
				FastLED.setBrightness(brightness);

				FastLED.show();
			}
			pixelMap.file.seek(4);
		}
		logn("completed play");
	}
	logn("file list completed");

	//DoScriptedDisplay();
}
