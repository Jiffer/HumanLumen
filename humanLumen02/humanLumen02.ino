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
AudioPlaySdWav           playWav1;     //xy=175,181
AudioMixer4              mixerR;         //xy=405,276
AudioMixer4              mixerL;         //xy=408,195
AudioOutputI2S           i2s1;           //xy=588,230
AudioConnection          patchCord1(playWav2, 0, mixerL, 1);
AudioConnection          patchCord2(playWav2, 1, mixerR, 1);
AudioConnection          patchCord3(playWav1, 0, mixerL, 0);
AudioConnection          patchCord4(playWav1, 1, mixerR, 0);
AudioConnection          patchCord5(mixerR, 0, i2s1, 1);
AudioConnection          patchCord6(mixerL, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=240,425
// GUItool: end automatically generated code

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

int fadeOnTime = 3000;
int fadeOffTime = 3000;
unsigned long printTimer = 0;

///////////////////////////////////////////
// LED setup
///////////////////////////////////////////
#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>
#define NUM_LEDS 150
#define NUM_RED 75
#define NUM_LAUGH 75
#define DATA_PIN 29
CRGB leds[NUM_LEDS];
// todo: make brightness an array to keep track of whole strip
static uint8_t redBrightness = 0;
//////////////////////////////////////////
// TF MINI setup
//////////////////////////////////////////
// using the hideakitai tfmini_arduino library (I think)
#include "TFmini.h"
TFmini tfmini;

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
  Serial8.begin(TFmini::DEFAULT_BAUDRATE);
  tfmini.attach(Serial8); // Teensy 4.1 pins 34, 35
  detector = HumanDetector();

  // Audio setup
  AudioMemory(16);
  sgtl5000_1.enable();
  sgtl5000_1.volume(1.0);

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

  if (tfmini.available())
  {
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
        if (fader1.fadeLevel < fader1.maxLevel) {
          fader1.startFadeOn(fadeOnTime);
        }
        if (fader2.fadeLevel < fader1.maxLevel) {
          fader2.startFadeOn(fadeOnTime);
        }
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
  if (millis() > printTimer) {
    Serial.print("f1: "); Serial.println(fader1.fadeLevel);
    Serial.println(fader2.fadeLevel);
    printTimer = millis() + 200;
  }

  mixerL.gain(0, fader1.fadeLevel); // track 1 L
  mixerL.gain(1, fader2.fadeLevel); // track 2 L
  mixerR.gain(0, fader1.fadeLevel); // track 1 R
  mixerR.gain(1, fader2.fadeLevel); // track 2 R

  //  if(fadeOff.fadeTimerActive){
  //    fadeOff.updateFade();
  //  }

  // light animations...
  //  if (detector.state == UPCLOSE) {
  // pixels2 crazy time
  unsigned long currentTime = millis();
  for (int i = NUM_RED; i < NUM_LAUGH + NUM_RED; i++) {
    // Set the i'th led to red
    if (random(100) < 3) {

      leds[i] = CHSV(fader2.fadeLevel * random(255), fader2.fadeLevel * random(255), fader2.fadeLevel * random(255));
    }
  }
  FastLED.show();
  //  }
  //  if (detector.state == DISTANT || detector.state == UPCLOSE) {
  currentTime = millis();
  for (int i = 0; i < NUM_RED; i++) {
    // Set the i'th led to red
    redBrightness = 155 + 65 * sin((currentTime + i * 5) * 2 * PI / 10000) + 35 * sin((currentTime + i * 5) * 2 * PI / 1200);
    leds[i] = CHSV(100, 255, fader1.fadeLevel * redBrightness);
  }
  FastLED.show();
  // }
  //  if (detector.state == NOPE) {
  //    unsigned long currentTime = millis();
  //    for (int i = 0; i < NUM_LAUGH + NUM_RED; i++) {
  //      // Set the i'th led to red
  //      if (random(100) < 5) {
  //        leds[i] = CHSV(0, 0, 0);
  //      }
  //    }
  //    FastLED.show();
  //  }
}
