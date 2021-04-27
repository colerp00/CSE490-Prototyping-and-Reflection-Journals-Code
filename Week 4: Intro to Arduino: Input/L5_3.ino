const int OUTPUT_GRN_LED_PIN = 3;
const int OUTPUT_BLU_LED_PIN = 6;
const int OUTPUT_WHT_LED_PIN = 9;
const int OUTPUT_YLW_LED_PIN = 10;
const int OUTPUT_RED_LED_PIN = 11;
const int OUTPUT_PIEZO_PIN = 2;
const int INPUT_FSR_PIN = A0;
const int INTERVAL_MS = 50;

const int MAX_ANALOG_OUT = 255;

unsigned long _lastTS = 0;

class LED {
  private:
    const int _outputPin;
  
  public:
    LED(int out) : _outputPin(out) {  // Constructor
      pinMode(_outputPin, OUTPUT);
    }

    void fade(int fadeVal) {
      analogWrite(_outputPin, fadeVal);
    }
};

LED _whiteLEDButton(OUTPUT_WHT_LED_PIN);
LED _redLEDButton(OUTPUT_RED_LED_PIN);
LED _yellowLEDButton(OUTPUT_YLW_LED_PIN);
LED _greenLEDButton(OUTPUT_GRN_LED_PIN);
LED _blueLEDButton(OUTPUT_BLU_LED_PIN);

void setup() {
  pinMode(OUTPUT_PIEZO_PIN, OUTPUT);
  pinMode(INPUT_FSR_PIN, INPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned long curTS = millis();
  
  int fsrVal = analogRead(INPUT_FSR_PIN);
  int piezoVal = map(fsrVal, 0, 1023, 50, 1500);
  int fadeVal = map(fsrVal, 0, 1023, 0, 255);

  if(curTS - _lastTS >= INTERVAL_MS && fsrVal > 10) {
    tone(OUTPUT_PIEZO_PIN, piezoVal);
    fadeLEDs(fadeVal);
  } else if (curTS - _lastTS >= INTERVAL_MS) {
    noTone(OUTPUT_PIEZO_PIN);
    fadeLEDs(0);
  }
  /*Serial.print(fsrVal);
  Serial.print(", ");
  Serial.print(fadeVal);
  Serial.print(", ");
  Serial.println(piezoVal);*/
}

void fadeLEDs(int fadeVal) {
  // f(x) = 111 (2 - |x - fadeVal/51|) where x = the LED (2,2,75,3.5,4.25,5)
  int greenFade = calcFadeVal(1, fadeVal);
  int blueFade = calcFadeVal(2, fadeVal);
  int whiteFade = calcFadeVal(3, fadeVal);
  int yellowFade = calcFadeVal(4, fadeVal);
  int redFade = calcFadeVal(5, fadeVal);
  
  _greenLEDButton.fade(greenFade);
  _blueLEDButton.fade(blueFade);
  _whiteLEDButton.fade(whiteFade);
  _yellowLEDButton.fade(yellowFade);
  _redLEDButton.fade(redFade);

  Serial.print(greenFade);
  Serial.print(", ");
  Serial.print(blueFade);
  Serial.print(", ");
  Serial.print(whiteFade);
  Serial.print(", ");
  Serial.print(yellowFade);
  Serial.print(", ");
  Serial.println(redFade);
}

double calcFadeVal(int ledNum, int fadeVal) {
  double result = 111 * (2 - abs((2 + ((ledNum-1) * 0.75)) - (fadeVal / (222 / 5))));
  if(result < 0) {
    result = 0;
  }
  return result;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
