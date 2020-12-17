#define CLOSE_DISTANCE 130
#define LONG_DISTANCE 400
#define ONTIME 1500
#define OFFTIME 4000

enum DETECT_STATE {
  NOPE,
  DISTANT,
  UPCLOSE
};

class HumanDetector {
    unsigned long changeStateTimer;
    bool stateTimerActive = true;
    uint16_t distance = 0;
    uint16_t strength = 0;
    uint16_t rangeType = 0;

  public:
    DETECT_STATE state;
    DETECT_STATE lastState;


    HumanDetector() {
      state = NOPE;
      lastState = NOPE;
      changeStateTimer = millis();

    }
    bool checkIt() {

      bool stateChanged = false;
      if (readDistance(tfmini) == true)
      {
        if (valid_data == true) {

          DETECT_STATE rawState = getRawState();
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
            else if (state == rawState) {
              if (stateTimerActive) {
                stateTimerActive = false;
              }
            }
            else {
              stateChanged = true;
              state = rawState;
            }
          }
        }
      }
      return stateChanged;
    }

    ////////////////////////////
    /// CALCULATE raw location
    ////////////////////////////
    DETECT_STATE getRawState() {
      DETECT_STATE tempState = NOPE;
      if (distance < CLOSE_DISTANCE) {
        tempState = UPCLOSE;
      } else if (distance < LONG_DISTANCE) {
        tempState = DISTANT;
      }
      else if (distance >= LONG_DISTANCE) {
        tempState = NOPE;
      }
      return tempState;
    }

    ////////////////////////////
    // I2c read TF Mini
    ////////////////////////////
    boolean readDistance(uint8_t deviceAddress)
    {
      Wire2.beginTransmission(deviceAddress);
      Wire2.write(0x01); //MSB
      Wire2.write(0x02); //LSB
      Wire2.write(7); //Data length: 7 bytes for distance data
      if (Wire2.endTransmission(false) != 0) {
        return (false); //Sensor did not ACK
      }
      Wire2.requestFrom(deviceAddress, (uint8_t)7); //Ask for 7 bytes

      if (Wire2.available())
      {
        for (uint8_t x = 0 ; x < 7 ; x++)
        {
          uint8_t incoming = Wire2.read();

          if (x == 0)
          {
            //Trigger done
            if (incoming == 0x00)
            {
              //Serial.print("Data not valid: ");//for debugging
              valid_data = false;
              //return(false);
            }
            else if (incoming == 0x01)
            {
              valid_data = true;
            }
          }
          else if (x == 2)
            distance = incoming; //LSB of the distance value "Dist_L"
          else if (x == 3)
            distance |= incoming << 8; //MSB of the distance value "Dist_H"
          else if (x == 4)
            strength = incoming; //LSB of signal strength value
          else if (x == 5)
            strength |= incoming << 8; //MSB of signal strength value
          else if (x == 6)
            rangeType = incoming; //range scale
        }
      }
      else
      {
        Serial.println("No wire data avail");
        return (false);
      }

      return (true);
    }

};
