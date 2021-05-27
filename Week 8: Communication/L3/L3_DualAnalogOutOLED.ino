#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int DELAY_MS = 5;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int BLU_ANALOG_INPUT_PIN = A0;
const int RED_ANALOG_INPUT_PIN = A1;
const int MAX_ANALOG_INPUT = 1023;

int _lastBluAnalogVal = -1;
int _lastRedAnalogVal = -1;
float _curShapeSizeFraction = -1;

// If false, only sends new data when the new analog value does not
// equal the last analog value. If true, always sends the data
boolean _alwaysSendData = true; 

const int MIN_SHAPE_SIZE = 4;
int MAX_SHAPE_SIZE;
const float MIN_ORIENT_VAL = 0.0;
const float MAX_ORIENT_VAL = 2 * PI; //6.2832; //6.28318530717958647693 = 2*pi

void setup() {
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  MAX_SHAPE_SIZE = min(_display.width(), _display.height()) / sin(PI / 6);

  _display.clearDisplay();
  _display.setTextSize(1);      // Normal 1:1 pixel scale
  _display.setTextColor(SSD1306_WHITE); // Draw white text
  _display.setCursor(0, 0);     // Start at top-left corner
}

void loop() {
  _display.clearDisplay();

  int bluAnalogVal = analogRead(BLU_ANALOG_INPUT_PIN);
  int redAnalogVal = analogRead(RED_ANALOG_INPUT_PIN);
  int shapeSize = map(bluAnalogVal, 0, MAX_ANALOG_INPUT, MIN_SHAPE_SIZE, MAX_SHAPE_SIZE);
  float shapeAngle = angleMap(redAnalogVal, 0.0, MAX_ANALOG_INPUT, MIN_ORIENT_VAL, MAX_ORIENT_VAL);
  int radius = shapeSize / 2;
  int xCenter = _display.width() / 2;
  int yCenter = _display.height() / 2;

  int xTip = getX(xCenter, radius, shapeAngle);
  int yTip = getY(yCenter, radius, shapeAngle);

  float rightAngle = getAngle(shapeAngle + (2 * PI / 3));
  int xRight = getX(xCenter, radius, rightAngle);
  int yRight = getY(yCenter, radius, rightAngle);

  float leftAngle = getAngle(shapeAngle + (4 * PI / 3));
  int xLeft = getX(xCenter, radius, leftAngle);
  int yLeft = getY(yCenter, radius, leftAngle);

  _display.fillTriangle(xTip, yTip, xRight, yRight, xLeft, yLeft, SSD1306_WHITE);
  //_display.fillCircle(xCenter, yCenter, radius, SSD1306_WHITE);
  _display.display();

  // If one of the analog values has changed, send the new ones over serial
  if(_alwaysSendData || _lastBluAnalogVal != bluAnalogVal || _lastRedAnalogVal != redAnalogVal){
    float bluSizeFrac = bluAnalogVal / (float)MAX_ANALOG_INPUT;
    float redSizeFrac = redAnalogVal / (float)MAX_ANALOG_INPUT;
    //Serial.println(bluSizeFrac, 4); // 4 decimal point precision
    //Serial.println(redSizeFrac, 4);
    Serial.println(String(bluSizeFrac, 4) + "," + String(redSizeFrac, 4));
  }

  _lastBluAnalogVal = bluAnalogVal;
  _lastRedAnalogVal = redAnalogVal;
  delay(DELAY_MS);
}

// map() uses integer map, so this method is used instead in order to map to a
// floating-point number for the angle of the tip of the triangle to south
float angleMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int getX(int xCenter, int radius, float angle) {
  return xCenter - (radius * sin(angle));
}

int getY(int yCenter, int radius, float angle) {
  return yCenter + (radius * cos(angle));
}

float getAngle(float angle) {
  if(angle >= MAX_ORIENT_VAL) {
    return angle - MAX_ORIENT_VAL;
  } else {
    return angle;
  }
}
