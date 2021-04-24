const int RGB_RED_PIN = 6;
const int RGB_BLU_PIN = 5;
const int RGB_GRN_PIN = 3;
const int MAX_COLOR_VALUE = 255;
const int FADE_STEP = 5;
const int DELAY_MS = 20;

int _rgbLedValues[] = {255, 0, 0}; // Red, Green, Blue

enum RGB{
  RED,
  GREEN,
  BLUE,
  NUM_COLORS
};

enum RGB _curFadingUpColor = GREEN;
enum RGB _curFadingDownColor = RED;
enum RGB _curFadingUpContenders[] = {RED, BLUE};

void setup() {
  // put your setup code here, to run once:
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GRN_PIN, OUTPUT);
  pinMode(RGB_BLU_PIN, OUTPUT);

  Serial.begin(9600);
  Serial.println("Red, Green, Blue");

  setColor(_rgbLedValues[RED], _rgbLedValues[GREEN], _rgbLedValues[BLUE]);
  delay(DELAY_MS);
}

void loop() {
  // put your main code here, to run repeatedly:
  _rgbLedValues[_curFadingUpColor] += FADE_STEP;
  _rgbLedValues[_curFadingDownColor] -= FADE_STEP;

  if(_rgbLedValues[_curFadingDownColor] < 0) {
    _rgbLedValues[_curFadingDownColor] = 0;
    enum RGB prevFadingDownColor = _curFadingDownColor;
    _curFadingDownColor = _curFadingUpColor;
    

    //if(_curFadingDownColor > (int)BLUE) {
    //  _curFadingDownColor = RED;
    //}
  }

  if(_rgbLedValues[_curFadingUpColor] > MAX_COLOR_VALUE) {
    _rgbLedValues[_curFadingUpColor] = MAX_COLOR_VALUE;
    enum RGB newFadingUpColor = setFadingUpColor();
    if(_curFadingUpContenders[0] == newFadingUpColor) {
      _curFadingUpContenders[0] = _curFadingDownColor;
    } else {
      _curFadingUpContenders[1] = _curFadingDownColor;
    }
    _curFadingUpColor = newFadingUpColor;
    //if(_curFadingUpColor > (int)BLUE) {
    //  _curFadingUpColor = RED;
    //}
  }

  setColor(_rgbLedValues[RED], _rgbLedValues[GREEN], _rgbLedValues[BLUE]);
  delay(DELAY_MS);
}

void setColor(int red, int green, int blue)
{
  Serial.print(red);
  Serial.print(", ");
  Serial.print(green);
  Serial.print(", ");
  Serial.println(blue);

  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GRN_PIN, green);
  analogWrite(RGB_BLU_PIN, blue);  
}

enum RGB setFadingUpColor() {
  unsigned long curTsMs = millis();
  return _curFadingUpContenders[curTsMs % 2];
}
