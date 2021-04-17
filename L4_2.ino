const int LED_OUTPUT_PIN = 3;
const int MAX_ANALOG_OUT = 255; // the max analog output on the Uno is 255
const int DELAY_MS = 5;

int _fadeAmount = 1;
int _curBrightness = 0;
int _cycleIndicator = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_OUTPUT_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(LED_OUTPUT_PIN, _curBrightness);

  _curBrightness = _curBrightness + _fadeAmount;

  if (_curBrightness <= 0 || _curBrightness >= MAX_ANALOG_OUT) {
    if(_cycleIndicator == 2) {
      _cycleIndicator = 0;
      if(_fadeAmount == 1) {
        _fadeAmount = 3;
      } else {
        _fadeAmount = 1;
      }
    }
    _fadeAmount = -_fadeAmount;
    _cycleIndicator += 1;
  }
  delay(DELAY_MS);
}
