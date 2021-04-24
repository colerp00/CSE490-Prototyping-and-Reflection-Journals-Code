#include "src/RGBConverter/RGBConverter.h"

const int RGB_RED_PIN = 6;
const int RGB_GRN_PIN  = 5;
const int RGB_BLU_PIN  = 3;
const int DELAY_MS = 20; // interval (ms) between incrementing hues
const byte MAX_RGB_VALUE = 255;

float _hue = 0; // varies between 0 - 1
float _step = 0.001f;

RGBConverter _rgbConverter;

void setup() {
  // Set the RGB pins to output
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GRN_PIN, OUTPUT);
  pinMode(RGB_BLU_PIN, OUTPUT);

  // Turn on Serial so we can verify expected colors via Serial Monitor
  Serial.begin(9600);   
}

void loop() {
  byte rgb[3];
  _rgbConverter.hslToRgb(_hue, 1, 0.5, rgb);

  // Print for debugging
  Serial.print("hue=");
  Serial.print(_hue);
  Serial.print(" r=");
  Serial.print(rgb[0]);
  Serial.print(" g=");
  Serial.print(rgb[1]);
  Serial.print(" b=");
  Serial.println(rgb[2]);
  
  setColor(rgb[0], rgb[1], rgb[2]); 

  // update hue based on step size
  _hue += _step;

  // if > 1, reset to 0
  if(_hue > 1.0){
    _hue = 0;
  }

  delay(DELAY_MS);
}

void setColor(int red, int green, int blue)
{
  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GRN_PIN, green);
  analogWrite(RGB_BLU_PIN, blue);  
}
