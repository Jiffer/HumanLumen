//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Human Lumen - Gail Folwell
//
// Jiffer Harriman
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////
// Teensy Audio player
///////////////////////////////////////////
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
// TeensyAudio GUItool: begin automatically generated code
AudioPlaySdWav           playWav2;     //xy=173,262
AudioPlaySdWav           playWav1;     //xy=176,181
AudioMixer4              mixerR;         //xy=406,275
AudioMixer4              mixerL;         //xy=408,195
AudioOutputI2S           i2s1;           //xy=588,230
AudioAnalyzePeak         peak1;          //xy=588,276
AudioConnection          patchCord1(playWav2, 0, mixerL, 1);
AudioConnection          patchCord2(playWav2, 1, mixerR, 1);
AudioConnection          patchCord3(playWav1, 0, mixerL, 0);
AudioConnection          patchCord4(playWav1, 1, mixerR, 0);
AudioConnection          patchCord5(mixerR, 0, i2s1, 1);
AudioConnection          patchCord6(mixerR, peak1);
AudioConnection          patchCord7(mixerL, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=240,425
// GUItool: end automatically generated code
elapsedMillis fps;
int laughPeak = 0;

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

int fadeOnTime = 4000;
int fadeOffTime = 4000;
unsigned long printTimer = 0;
//unsigned long laughTimer = 0;

///////////////////////////////////////////
// LED setup
///////////////////////////////////////////
#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>
#define NUM_LEDS 300

#define DATA_PIN 29
CRGB leds[NUM_LEDS];
// todo: make brightness an array to keep track of whole strip
static uint8_t redBrightness = 0;
//////////////////////////////////////////
// TF MINI setup
//////////////////////////////////////////
boolean valid_data = false; //ignore invalid ranging data
const byte tfmini = 0x10; //TFMini I2C Address

////////////////////////////////////////////////
// my headers
////////////////////////////////////////////////
#include "detector.h"
#include "readDirectory.h"
#include "player.h"

HumanDetector detector;
Fader fadeOff;
Fader fader1;
Fader fader2;

////////////////////////////////////////////////
////////////////////////////////////////////////
// Setup
////////////////////////////////////////////////
////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  // TFmini Setup
  Wire2.begin();
  detector = HumanDetector();

  // Audio setup
  AudioMemory(16);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  mixerL.gain(0, 0); // track 1 L
  mixerL.gain(1, 0); // track 2 L
  mixerR.gain(0, 0); // track 1 R
  mixerR.gain(1, 0); // track 2 R


  // SD Card
  root = SD.open("/");
  readDirectory(root, 0, -1);

  Serial.println("finished reading directories");
  for (int i = 0; i < 8; i++) {
    Serial.print("tracks in folder ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(numTracks[i]);
  }

  // initialize animation
  LEDS.addLeds<WS2812SERIAL, DATA_PIN, RGB>(leds, NUM_LEDS);
  LEDS.setBrightness(255);
  delay(200);
  Serial.println("go to main");

  fadeOff = Fader();
  fader1 = Fader();
  fader2 = Fader();
}

//////////////////////////////////////
//////////////////////////////////////
// Main loop
//////////////////////////////////////
//////////////////////////////////////
void loop() {

  if (!playWav1.isPlaying()) {
    playFile1();
  }
  if (!playWav2.isPlaying()) {
    playFile2();
  }

  bool stateChanged = detector.checkIt();
  if (stateChanged) {
    Serial.print("new state: ");
    if (detector.state == NOPE) {
      if (fader1.fadeLevel > 0) {
        fader1.startFadeOff(fadeOffTime);
      }
      if (fader2.fadeLevel > 0) {
        fader2.startFadeOff(fadeOffTime);
      }
    }
    else if (detector.state == DISTANT) {
      Serial.println("DISTANT");
      if (fader1.fadeLevel < fader1.maxLevel) {
        fader1.startFadeOn(fadeOnTime);
      }
      if (fader2.fadeLevel > 0) {
        fader2.startFadeOff(fadeOffTime);
      }
    }
    else if (detector.state == UPCLOSE) {
      Serial.println("UPCLOSE");
      if (fader1.fadeLevel > 0) {
        fader1.startFadeOff(fadeOffTime);
      }
      if (fader2.fadeLevel < fader2.maxLevel) {
        fader2.startFadeOn(fadeOnTime);
      }
    }
  }


  if (fader1.fadeState == FADE_ON) {
    fader1.updateFadeOn();
  } else if (fader1.fadeState == FADE_OFF) {
    fader1.updateFadeOff();
  }
  if (fader2.fadeState == FADE_ON) {
    fader2.updateFadeOn();
  } else if (fader2.fadeState == FADE_OFF) {
    fader2.updateFadeOff();
  }
  

  mixerL.gain(0, fader1.fadeLevel); // track 1 L
  mixerL.gain(1, fader2.fadeLevel); // track 2 L
  mixerR.gain(0, fader1.fadeLevel); // track 1 R
  mixerR.gain(1, fader2.fadeLevel); // track 2 R

  // print fade values
  bool printer = false;
  if (millis() > printTimer) {
    Serial.print("f1: "); Serial.println(fader1.fadeLevel);
    Serial.println(fader2.fadeLevel);
    printTimer = millis() + 500;
    printer = true;
  }

  
  unsigned long currentTime = millis();
  if (fps > 15) {
    if (peak1.available()) {
      fps = 0;
      if(255*peak1.read() > laughPeak){
        laughPeak = 255*peak1.read();
      }else{
        laughPeak *= .999;
      }
      if(laughPeak > 255){
        laughPeak = 255;
      }
    }
  }
  for (int i = 0; i < NUM_LEDS; i++) {
    redBrightness = 125 + 65 * sin((currentTime + i * 5) * 2 * PI / 10000) + 25 * sin((currentTime + i * 5) * 2 * PI / 1200);
    int onBrightness =  25 + 24 * sin((currentTime + i * 50) * 2 * PI / 5000);
    int h = (int)(98 + fader2.fadeLevel * random(255)) % 255;
    int s = 255;
    int b = ((redBrightness * fader1.fadeLevel + (fader1.maxLevel-fader1.fadeLevel)*onBrightness + laughPeak*fader2.fadeLevel) / fader1.maxLevel);
    if(b > 255){
      b = 255;
    }
    leds[i] = CHSV(h, s, b);
  }
  FastLED.show();

}
