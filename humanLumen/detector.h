enum DETECT_STATE {
    NOPE,
    DISTANT,
    UPCLOSE
  };

class HumanDetector{
  unsigned long changeStateTimer;
  
  public:
  DETECT_STATE state;

  HumanDetector(){
    state = NOPE;
    changeStateTimer = 0;
  }
  bool checkIt(){
    //    Serial.print("distance : ");
    //    Serial.println(tfmini.getDistance());
    //    Serial.print("strength : ");
    //    Serial.println(tfmini.getStrength());
    //    Serial.print("int time : ");
    //    Serial.println(tfmini.getIntegrationTime());  
    bool stateChanged = false;
    int currentReading = tfmini.getDistance();
    if(currentReading < CLOSE_DISTANCE){
      if(state != UPCLOSE){
        state = UPCLOSE;
        stateChanged = true;
        Serial.print("close distance: ");
        Serial.println(currentReading);
      }
    }else if(currentReading < LONG_DISTANCE){
      if(state != DISTANT){
        state = DISTANT;
        stateChanged = true;
        Serial.print("long distance: ");
        Serial.println(currentReading);
      }
    }
    else if(currentReading >= LONG_DISTANCE){
      if(state != NOPE){
        state = NOPE;
        stateChanged = true;
        Serial.print("no ones home: ");
        Serial.println(currentReading);
      }
    }
    return stateChanged;
  }
};
