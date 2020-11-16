enum FADE_STATE {
  NOPE_DISTANT,
  DISTANT_NOPE,
  NOPE_CLOSE,
  CLOSE_NOPE,
  DISTANT_CLOSE,
  CLOSE_DISTANT,
  DONE
};

class Fader {
  public:
    float fadeLevel;
    FADE_STATE fadeState;
    unsigned long fadeTimer;
    int fadeTime;
    bool fadeTimerActive;

    Fader() {
      fadeState = DONE;
      fadeLevel = 0;
      fadeTimer = 0;
      fadeTimerActive = false;
    }

    startFadeOff(int _fadeTime) {
      Serial.println("starting fade");
      fadeLevel = 1.0;
      fadeTimer = millis() + _fadeTime;
      fadeTime = _fadeTime;
      fadeTimerActive = true;
      Serial.println(fadeTimer);
    }

    updateFade() {
      if (fadeTimerActive) {
        if (fadeTimer > millis()) {
          float currentVal = (float)(fadeTimer - millis()) / (float)fadeTime;
          fadeLevel = currentVal;
        }
        else {
          Serial.println("fade timer expired");
          fadeLevel = 0;
          fadeTimerActive = false;
        }
      }
    }
};
