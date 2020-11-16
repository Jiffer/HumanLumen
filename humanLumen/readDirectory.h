////////////////////////////////////
////////////////////////////////////
//
// readDirectory()
//
////////////////////////////////////
const int numButtons = 8; // number of track lists

File root;

char fileNameList[100][16]; // currently a max of 100 tracks
char dirNameList[100][16];
char fList[numButtons][100][32]; // file location including directory name and filename.wav
int numTracks[numButtons] = {0, 0, 0, 0, 0, 0, 0, 0};
int currentTrack[numButtons] = {0, 0, 0, 0, 0, 0, 0, 0};
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
