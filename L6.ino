const int RGB_RED_LED_PIN = 6;
const int RGB_BLU_LED_PIN = 5;
const int RGB_GRN_LED_PIN = 3;
const int MAX_ANALOG_OUT = 255;
const int DELAY_MS = 50;

int _redFadeAmount = 1;
int _blueFadeAmount = 0;
int _greenFadeAmount = -1;
int _curRedBrightness = 255;
int _curBlueBrightness = 0;
int _curGreenBrightness = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(RGB_RED_LED_PIN, OUTPUT);
  pinMode(RGB_BLU_LED_PIN, OUTPUT);
  pinMode(RGB_GRN_LED_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  _curRedBrightness = _curRedBrightness + _redFadeAmount;
  _curGreenBrightness = _curGreenBrightness + _greenFadeAmount;
  _curBlueBrightness = _curBlueBrightness + _blueFadeAmount;
   
  analogWrite(RGB_RED_LED_PIN, _curRedBrightness);
  analogWrite(RGB_GRN_LED_PIN, _curGreenBrightness);
  analogWrite(RGB_BLU_LED_PIN, _curBlueBrightness);
  if(_curRedBrightness >= MAX_ANALOG_OUT || _curGreenBrightness <= 0) {
    _redFadeAmount = -1;
    _greenFadeAmount = 1;
    _blueFadeAmount = 0;
  } else if(_curGreenBrightness >= MAX_ANALOG_OUT || _curBlueBrightness <= 0) {
    _redFadeAmount = 0;
    _greenFadeAmount = -1;
    _blueFadeAmount = 1;
  } else if(_curBlueBrightness >= MAX_ANALOG_OUT || _curRedBrightness <= 0) {
    _redFadeAmount = 1;
    _greenFadeAmount = 0;
    _blueFadeAmount = -1;
  }
  delay(DELAY_MS);
}
