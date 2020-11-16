enum FADE_STATE {
  FADE_OFF,
  FADE_ON,
  DONE
};

class Fader {
  public:
    float fadeLevel;
    FADE_STATE fadeState;
    unsigned long fadeTimer;
    int fadeTime;
    bool fadeTimerActive;
    float maxLevel = .8;

    Fader() {
      fadeState = DONE;
      fadeLevel = 0;
      fadeTimer = 0;
      fadeTimerActive = false;
    }

    void startFadeOff(int _fadeTime) {
      Serial.println("starting fade off");
      fadeTimer = millis() + _fadeTime;
      fadeTime = _fadeTime;
      fadeTimerActive = true;
      fadeState = FADE_OFF;
    }

    void startFadeOn(int _fadeTime) {
      Serial.println("starting fade on");
      fadeTimer = millis() + _fadeTime;
      fadeTime = _fadeTime;
      fadeTimerActive = true;
      fadeState = FADE_ON;
    }

    void updateFadeOff() {
      if (fadeTimerActive) {
        if (fadeTimer > millis()) {
          float currentVal = maxLevel * (float)(fadeTimer - millis()) / (float)fadeTime;
          if(currentVal < 0){
            currentVal = 0;
          }
          fadeLevel = currentVal;
        }
        else {
          Serial.println("fade timer expired");
          fadeLevel = 0;
          fadeTimerActive = false;
          fadeState = DONE;
        }
      }
    }

    updateFadeOn() {
      if (fadeTimerActive) {
        if (fadeTimer > millis()) {
          float currentVal =maxLevel - maxLevel*(float)(fadeTimer - millis()) / (float)fadeTime;
          if(currentVal > maxLevel){
            currentVal = maxLevel;
          }
          fadeLevel = currentVal;
        }
        else {
          Serial.println("fade timer expired");
          fadeLevel = maxLevel;
          fadeTimerActive = false;
          fadeState = DONE;
        }
      }
    }
};
