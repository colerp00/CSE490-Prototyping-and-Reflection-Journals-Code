#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "src/Ships/Ships.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const int JOYSTICK_UPDOWN_PIN = A1;
const int JOYSTICK_LEFTRIGHT_PIN = A0;

const int MAX_ANALOG_VAL = 1023;
const enum JoystickYDirection JOYSTICK_Y_DIR = RIGHT;

// Analog joystick
ParallaxJoystick _analogJoystick(JOYSTICK_UPDOWN_PIN, JOYSTICK_LEFTRIGHT_PIN, MAX_ANALOG_VAL, JOYSTICK_Y_DIR);

const int SHOOT_BUTTON_INPUT_PIN = 6;
const int SHIELD_BUTTON_INPUT_PIN = 7;

// Player-controlled triangle "ship"
uint16_t _playerShipCoords[] = {20, 32}

const int MOVE_DELAY = 10;
int _numLoops = 0;

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  _display.clearDisplay();
}

void loop() {
  _display.clearDisplay();

  // Read analog joystick to control player's ship
  _analogJoystick.read();
  int upDownVal = _analogJoystick.getUpDownVal();
  int leftRightVal = _analogJoystick.getLeftRightVal();

  // Translate joystick movement into amount of pixels to move
  int yMovementPixels = map(upDownVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -4, 5);
  int xMovementPixels = map(leftRightVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -10, 11);

  // Set the new location of the player's ship based on joystick position
  _playerBall.setLocation(_playerBall.getX() + xMovementPixels, _playerBall.getY() - yMovementPixels);

  // Don't let the player's ship outside of the screen area
  _playerBall.forceInside(0, 0, _display.width(), _display.height());
  
  // Draw the player's ship
  drawPlayerShip(_playerShipCoords[0], _playerShipCoords[1]);
  if(_numLoops == MOVE_DELAY) {
    _playerShipCoords[0]++;
    //_playerShipCoords[1]++;
    _numLoops = 0;
    Serial.print(_playerShipCoords[0]);
    Serial.print(",");
    Serial.println(_playerShipCoords[1]);
  } else {
    _numLoops++;
    Serial.println(_numLoops);
  }
}

void drawPlayerShip(uint16_t x, uint16_t y) {
  //fillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
  fillTriangle(_playerShipCoords[0], _playerShipCoords[1],
               _playerShipCoords[0] - 10, _playerShipCoords[1] + 3,
               _playerShipCoords[0] - 10, _playerShipCoords[1] - 3,
               SSD1306_WHITE);
}
