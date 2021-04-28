const int RGB_RED_PIN = 9;
const int RGB_GRN_PIN = 6;
const int RGB_BLU_PIN = 3;
// For mode 1
const int PHOTOCELL_INPUT_PIN = A0;
const int MIN_PHOTOCELL_VAL = 0;
const int MAX_PHOTOCELL_VAL = 60;

// For mode 2
const int INPUT_DIAL_PIN = A1;
const int MIN_DIAL_VAL = 0;
const int MAX_DIAL_VAL = 1023;

// For mode 3
const int INPUT_FSR_PIN = A2;
const int MIN_FSR_VAL = 0;
const int MAX_FSR_VAL = 1023;
int _curColorMode = 0;
int _lastRecordedPushState = 0;

// mode switch button
const int INPUT_MODE_BUTTON_PIN = 2;
int _lastRecordedButtonState = LOW;

const int MAX_COLOR_VALUE = 255;

const int INTERVAL_MS = 30;
const int FADE_STEP = 1;

int _mode;  // 1,2,3 for modes 1,2,3 respectively
unsigned long _lastTimestamp;

int _rgbLedVals[] = {255, 0, 0}; // Red, Green, Blue

enum RGB {
  RED,
  GREEN,
  BLUE,
  NUM_COLORS
};

enum RGB _curFadingUpColor = GREEN;
enum RGB _curFadingDownColor = RED;



void setup() {
  Serial.begin(9600);

  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GRN_PIN, OUTPUT);
  pinMode(RGB_BLU_PIN, OUTPUT);
  
  pinMode(PHOTOCELL_INPUT_PIN, INPUT);
  pinMode(INPUT_DIAL_PIN, INPUT);
  pinMode(INPUT_FSR_PIN, INPUT);
  pinMode(INPUT_MODE_BUTTON_PIN, INPUT);
  _mode = 0;
  _lastTimestamp = 0;
  
  setColor(_rgbLedVals[RED], _rgbLedVals[GREEN], _rgbLedVals[BLUE]);
}

void loop() {
  unsigned long _curTimestamp = millis();

  if(_curTimestamp - _lastTimestamp >= INTERVAL_MS) {
    // Change the mode first
    int modeButtonVal = digitalRead(INPUT_MODE_BUTTON_PIN);
    if(modeButtonVal == HIGH && modeButtonVal != _lastRecordedButtonState) {
      _mode++;
      if(_mode > 3) {
        _mode = 1;
      }
    }
    _lastRecordedButtonState = modeButtonVal;
    Serial.println(_mode);

    if(_mode == 1) {
      mode1();
    } else if(_mode == 2) {
      mode2();
    } else if(_mode == 3) {
      mode3();
    }
  }
}

void mode1() {
  int photocellVal = analogRead(PHOTOCELL_INPUT_PIN);

  // Remap the value for output. 
  int ledScale = map(photocellVal, MIN_PHOTOCELL_VAL, MAX_PHOTOCELL_VAL, 0, 255);

  // Constrain output of the LED
  ledScale = constrain(ledScale, 0, 255);

  // Invert LED
  ledScale = 255 - ledScale;

  // Print the raw photocell value and the converted LED value
  /*Serial.print(photocellVal);
  Serial.print(",");
  Serial.println(ledScale);
  Serial.print(_rgbLedVals[_curFadingUpColor]);
  Serial.print(",");
  Serial.println(_rgbLedVals[_curFadingDownColor]);*/
  
  _rgbLedVals[_curFadingUpColor] += FADE_STEP;
  _rgbLedVals[_curFadingDownColor] -= FADE_STEP;

  if(_rgbLedVals[_curFadingUpColor] > MAX_COLOR_VALUE) {
    _rgbLedVals[_curFadingUpColor] = MAX_COLOR_VALUE;
    _curFadingUpColor = (RGB)((int)_curFadingUpColor + 1);

    if(_curFadingUpColor > (int)BLUE) {
      _curFadingUpColor = RED;
    }
  }

  if(_rgbLedVals[_curFadingDownColor] < 0) {
    _rgbLedVals[_curFadingDownColor] = 0;
    _curFadingDownColor = (RGB)((int)_curFadingDownColor + 1);

    if(_curFadingDownColor > (int)BLUE) {
      _curFadingDownColor = RED;
    }
  }
  
  int redScaled = map(_rgbLedVals[RED], 0, MAX_COLOR_VALUE, 0, ledScale);
  int greenScaled = map(_rgbLedVals[GREEN], 0, MAX_COLOR_VALUE, 0, ledScale);
  int blueScaled = map(_rgbLedVals[BLUE], 0, MAX_COLOR_VALUE, 0, ledScale);
  
  /*Serial.print(redScaled);
  Serial.print(", ");
  Serial.print(greenScaled);
  Serial.print(", ");
  Serial.println(blueScaled);*/
  
  setColor(redScaled, greenScaled, blueScaled);
}

void mode2() {
  int dialVal = analogRead(INPUT_DIAL_PIN);
  //Serial.println(dialVal);
  int dialScale = map(dialVal, MIN_DIAL_VAL, MAX_DIAL_VAL, 0, 510);

  int redVal = 255 - abs(510 - dialScale);
  if(redVal < 0) {
    redVal = 0;
  }
  int greenVal = 255 - abs(255 - dialScale);
  int blueVal = 255 - abs(0 - dialScale);
  if(blueVal < 0) {
    blueVal = 0;
  }

  setColor(redVal, greenVal, blueVal);
}

void mode3() {
  int fsrVal = analogRead(INPUT_FSR_PIN);
  //Serial.println(fsrVal);
  int recordedState = 0;
  if(fsrVal > 233) {
    recordedState = 1;
  }
  
  if(fsrVal >= 233 && _lastRecordedPushState == 0 && recordedState != _lastRecordedPushState) {
    _curColorMode++;
    if(_curColorMode > 6) {
      _curColorMode = 0;
    }
  }
  _lastRecordedPushState = recordedState;
  
  setColorCombo(_curColorMode);
}

void setColor(int red, int green, int blue) {
  /*Serial.print(red);
  Serial.print(", ");
  Serial.print(green);
  Serial.print(", ");
  Serial.println(blue);*/
  
  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GRN_PIN, green);
  analogWrite(RGB_BLU_PIN, blue);
}

void setColorCombo(int color) {
  if(color == 0) {
    setColor(255, 0, 0);
  } else if(color == 1) {
    setColor(205,50,0);
  } else if(color == 2) {
    setColor(128, 127, 0);
  } else if(color == 3) {
    setColor(0, 255, 0);
  } else if(color == 4) {
    setColor(0, 0, 255);
  } else if(color == 5) {
    setColor(127, 0, 128);
  } else if(color == 6) {
    setColor(255, 255, 255);
  }
}
