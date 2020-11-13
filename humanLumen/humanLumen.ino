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

///////////
// detector object
//////////
#define CLOSE_DISTANCE 60
#define LONG_DISTANCE 350
#define ONTIME 1000 // 1 second before turning on
#define OFFTIME 10000 // 10 seconds before
#include "detector.h"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////
const int numButtons = 8; // number of track lists

File root;

char fileNameList[100][16]; // currently a max of 100 tracks
char dirNameList[100][16];
char fList[numButtons][100][32]; // file location including directory name and filename.wav
int numTracks[numButtons] = {0, 0, 0, 0, 0, 0, 0, 0};
int currentTrack[numButtons] = {0, 0, 0, 0, 0, 0, 0, 0};
////////////////////////////////////////////////

HumanDetector detector;

////////////////////////////////////////////////
////////////////////////////////////////////////
// Setup
////////////////////////////////////////////////
////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Serial8.begin(TFmini::DEFAULT_BAUDRATE);
  tfmini.attach(Serial8); // Teensy 4.1 pins 34, 35
  Serial.println("start");

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
  root = SD.open("/");

  readDirectory(root, 0, -1);

  Serial.println("all done reading directories!");
  for (int i = 0; i < 8; i++) {
    Serial.print("tracks in folder ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(numTracks[i]);
  }

  mixerL.gain(0, .5); // track 1 L
  mixerL.gain(1, .5); // track 2 L
  mixerR.gain(0, .5); // track 1 R
  mixerR.gain(1, .5); // track 2 R

  detector = HumanDetector();

  // initialize animation
  LEDS.addLeds<WS2812SERIAL, DATA_PIN, RGB>(leds, NUM_LEDS);
  LEDS.setBrightness(255);
  delay(200);
  Serial.println("go to main");
}

//////////////////////////////////////
//////////////////////////////////////
// Main loop
//////////////////////////////////////
//////////////////////////////////////
void loop() {
  if (tfmini.available())
  {
    bool stateChanged = detector.checkIt();
    if (stateChanged) {
      Serial.print("new state");
      if (detector.state == NOPE) {
        Serial.println("NOPE");
        if (playWav1.isPlaying()) {
          playWav1.stop();
        }
        if (playWav2.isPlaying()) {
          playWav2.stop();
        }
      }
      else if (detector.state == DISTANT) {
        Serial.println("DISTANT");
        if (playWav2.isPlaying()) {
          playWav2.stop();
        }
        if (!playWav1.isPlaying()) {
          playFile1();
        }
      }
      else if (detector.state == UPCLOSE) {
        Serial.println("UPCLOSE");
        if (!playWav1.isPlaying()) {
          playFile1();
        }
        if (!playWav2.isPlaying()) {
          playFile2();
        }
      }
    }
  }
  // light animations...
  if (detector.state == UPCLOSE) {
    // pixels2 crazy time
    unsigned long currentTime = millis();
    for (int i = NUM_RED; i < NUM_LAUGH + NUM_RED; i++) {
      // Set the i'th led to red
      if (random(100) < 3) {
        int randomColor = random(255);
        leds[i] = CHSV(randomColor, random(255), random(255));
      }
    }
    FastLED.show();
  }
  if (detector.state == DISTANT || detector.state == UPCLOSE) {
    unsigned long currentTime = millis();
    for (int i = 0; i < NUM_RED; i++) {
      // Set the i'th led to red
      redBrightness = 155 + 65 * sin((currentTime + i * 5) * 2 * PI / 10000) + 35 * sin((currentTime + i * 5) * 2 * PI / 1200);
      leds[i] = CHSV(100, 255, redBrightness);
    }
    FastLED.show();
  }
  if(detector.state == NOPE){
    unsigned long currentTime = millis();
    for (int i = 0; i < NUM_LAUGH + NUM_RED; i++) {
      // Set the i'th led to red
      if (random(100) < 5) {        
        leds[i] = CHSV(0, 0, 0);
      }
    }
    FastLED.show();
  }
}

/////////////////////////////////////
/////////////////////////////////////
// playFile()
/////////////////////////////////////
/////////////////////////////////////
void playFile1()
{
  const char *filename = fList[0][currentTrack[0]];
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav1.play(filename);

  // A brief delay for the library read WAV info
  delay(5);
  currentTrack[0]++;
  currentTrack[0] %= numTracks[0];
}

void playFile2()
{
  const char *filename = fList[1][currentTrack[1]];
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav2.play(filename);

  // A brief delay for the library read WAV info
  delay(5);
  currentTrack[1]++;
  currentTrack[1] %= numTracks[1];
}

////////////////////////////////////
////////////////////////////////////
//
// readDirectory()
//
////////////////////////////////////
////////////////////////////////////
void readDirectory(File dir, int numTabs, int currentPassIn) {
  int currentFolder = currentPassIn;
  char* entryName;
  char* lastDirName;// = (char*)malloc(32*sizeof(char));  // unitialized perhaps because it may be called before entry.name()?
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      if (currentFolder != -1) {
        Serial.print("found " );
        Serial.print(numTracks[currentFolder]);
        Serial.print(" valid .wav files in currentFolder: ");
        Serial.println(currentFolder);
      }
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    entryName = entry.name();

    boolean _start = false;
    if (entryName[0] == '_') {
      _start = true;
    }
    if (!_start) {
      int startingAt = 0;
      for (int i = 0; i < 16; i++) {
        if (entryName[i] == '.') {
          startingAt = i;
        }
      }

      Serial.print(" cf is : ");
      Serial.println(currentFolder);
      if (currentFolder != -1) {
        if (startingAt > 0) {
          if (entryName[startingAt + 1] == 'W' && entryName[startingAt + 2] == 'A' && entryName[startingAt + 3] == 'V') {
            // put it in an array!
            for (int i = 0; i < 16; i++) {
              dirNameList[numTracks[currentFolder]][i] = lastDirName[i];
            }
            for (int i = 0; i < 16; i++) {
              fileNameList[numTracks[currentFolder]][i] = entryName[i];

            }

            char* dirName = strcat(strcat(dirNameList[numTracks[currentFolder]], "/"), fileNameList[numTracks[currentFolder]]);
            for (int i = 0; i < 32; i++) {
              fList[currentFolder][numTracks[currentFolder]][i] = dirName[i]; // todo! use folder name

            }

            Serial.print("stored file name: ");
            Serial.print(fileNameList[numTracks[currentFolder]]);
            Serial.print(" at loc: ");
            Serial.print(numTracks[currentFolder]);
            Serial.print(" whole ");
            Serial.println( fList[currentFolder][numTracks[currentFolder]] );
            numTracks[currentFolder]++;

          }
        }
      }
      Serial.print(entry.name());

      if (entry.isDirectory()) {
        Serial.println("/");

        lastDirName = entry.name();
        currentFolder = -1;
        if (lastDirName[0] == 'A' ) {
          Serial.println("current folder is 0");
          currentFolder = 0;
        } else if (lastDirName[0] == 'B' ) {
          Serial.println("current folder is 1");
          currentFolder = 1;
        } else if (lastDirName[0] == 'C' ) {
          Serial.println("current folder is 2");
          currentFolder = 2;
        } else if (lastDirName[0] == 'D' ) {
          Serial.println("current folder is 3");
          currentFolder = 3;
        } else if (lastDirName[0] == 'E' ) {
          currentFolder = 4;
        } else if (lastDirName[0] == 'F' ) {
          currentFolder = 5;
        } else if (lastDirName[0] == 'G' ) {
          currentFolder = 6;
        } else if (lastDirName[0] == 'H' ) {
          currentFolder = 7;
        }

        // call back into itself
        readDirectory(entry, numTabs + 1, currentFolder);
      } else {
        // files have sizes, directories do not
        Serial.print("\t\t");
        Serial.println(entry.size(), DEC);
      }
    }
    entry.close();
  }
}
