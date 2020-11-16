#define CLOSE_DISTANCE 60
#define LONG_DISTANCE 350
#define ONTIME 1500 
#define OFFTIME 5000 

enum DETECT_STATE {
  NOPE,
  DISTANT,
  UPCLOSE
};

class HumanDetector {
    unsigned long changeStateTimer;
    bool stateTimerActive = true;

  public:
    DETECT_STATE state;
    DETECT_STATE lastState;


    HumanDetector() {
      state = NOPE;
      lastState = NOPE;
      changeStateTimer = millis();

    }
    bool checkIt() {
      //    Serial.print("distance : ");
      //    Serial.println(tfmini.getDistance());
      //    Serial.print("strength : ");
      //    Serial.println(tfmini.getStrength());
      //    Serial.print("int time : ");
      //    Serial.println(tfmini.getIntegrationTime());

      bool stateChanged = false;
      int currentReading = tfmini.getDistance();

      DETECT_STATE rawState = getRawState(currentReading);
      if (rawState != state) {
        if ((state == NOPE || state == UPCLOSE) && rawState == DISTANT) {
          // check timer
          if (stateTimerActive) {
            if (changeStateTimer < millis()) {
              state = rawState;
              stateTimerActive = false;
              stateChanged = true;
            }
          } 
          else {
            changeStateTimer = millis() + ONTIME;
            Serial.print("set timer: "); Serial.println(changeStateTimer);
            stateTimerActive = true;
          }
        }
        else if ((state == NOPE || state == DISTANT) && rawState == UPCLOSE) {
          // check timer
          if (stateTimerActive) {
            if (changeStateTimer < millis()) {
              state = rawState;
              stateTimerActive = false;
              stateChanged = true;
            }
          } 
          else {
            changeStateTimer = millis() + ONTIME;
            Serial.print("set timer: "); Serial.println(changeStateTimer);
            stateTimerActive = true;
          }
        } else if ((state == DISTANT || state == UPCLOSE) && rawState == NOPE) {
          // check timer
          if (stateTimerActive) {
            if (changeStateTimer < millis()) {
              state = rawState;
              stateTimerActive = false;
              stateChanged = true;
            }
          } 
          else {
            changeStateTimer = millis() + OFFTIME;
            Serial.print("set timer: "); Serial.println(changeStateTimer);
            stateTimerActive = true;
          }
        }
        else if(state == rawState){
          if(stateTimerActive){
            stateTimerActive = false;
          }
        }
        else{
          stateChanged = true;
          state = rawState;
        }
      }
      return stateChanged;
    }

    DETECT_STATE getRawState(int currentReading) {
      DETECT_STATE tempState = NOPE;
      if (currentReading < CLOSE_DISTANCE) {
        tempState = UPCLOSE;
      } else if (currentReading < LONG_DISTANCE) {
        tempState = DISTANT;
      }
      else if (currentReading >= LONG_DISTANCE) {
        tempState = NOPE;
      }
      return tempState;
    }

};
