#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ParallaxJoystick.hpp>;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define DEBUG_1 false
#define DEBUG_2 true

Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int JOYSTICK_UPDOWN_PIN = A1;
const int JOYSTICK_LEFTRIGHT_PIN = A0;

const int MAX_ANALOG_VAL = 1023;
const enum JoystickYDirection JOYSTICK_Y_DIR = RIGHT;

// Analog joystick
ParallaxJoystick _analogJoystick(JOYSTICK_UPDOWN_PIN, JOYSTICK_LEFTRIGHT_PIN, MAX_ANALOG_VAL, JOYSTICK_Y_DIR);

// for tracking fps
float _fps = 0;
unsigned long _frameCount = 0;
unsigned long _fpsStartTimeStamp = 0;

// status bar
const boolean _drawStatusBar= true; // change to show/hide status bar
const int DELAY_LOOP_MS = 5;

typedef struct moving_tri {
  int _width;
  int _height;
  int _xSpeed;
  int _ySpeed;
  int _x;
  int _y;
} Tri;

Tri mover = {10, 20, 0, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};

void setup() {
  Serial.begin(9600);
  
  initializeOledAndShowStartupScreen();
  //_display.clearDisplay();
  drawThing();
}

void loop() {
  _display.clearDisplay();

  if(_drawStatusBar){
    drawStatusBar();
  }

  calcFrameRate();

  // Read analog joystick to control player ball
  _analogJoystick.read();
  int upDownVal = _analogJoystick.getUpDownVal();
  int leftRightVal = _analogJoystick.getLeftRightVal();

  // Translate joystick movement into amount of pixels to move
  int yMovementPixels = map(upDownVal, 0, _analogJoystick.getMaxAnalogValue() + 1, 10, -11);
  int xMovementPixels = map(leftRightVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -10, 11);

  mover._xSpeed = xMovementPixels;
  mover._ySpeed = yMovementPixels;
  
  mover._x += mover._xSpeed;
  mover._y += mover._ySpeed;

  if(DEBUG_2) {
    Serial.print(mover._x);
    Serial.print(",");
    Serial.print(mover._y);
    Serial.print(",");
    Serial.print(mover._xSpeed);
    Serial.print(",");
    Serial.println(mover._ySpeed);
  }

  moveBackToBounds();
  drawThing();

  _display.display();
}

void moveBackToBounds() {
  if(mover._x + 5 >= SCREEN_WIDTH) {
    mover._x = SCREEN_WIDTH - 6;
  } else if(mover._x - 5 <= 0) {
    mover._x = 5;
  }
  if(mover._y + 5 >= SCREEN_HEIGHT) {
    mover._y = SCREEN_HEIGHT - 6;
  } else if(mover._y - 5 <= 0) {
    mover._y = 5;
  }
}

/*void drawTri() {
  int xTip = mover._x * mover._xSpeed;
  int yTip = mover._y * (mover._height/2);
  //_display.fillTriangle(mover, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, SSD1306_WHITE);
}*/

void drawThing() {
  _display.fillCircle(mover._x, mover._y, 5, SSD1306_WHITE);
  _display.drawLine(mover._x, mover._y, mover._x + mover._xSpeed, mover._y + mover._ySpeed, SSD1306_WHITE);
  if(DEBUG_1) {
    Serial.println("drawThing");
    Serial.print(mover._x);
    Serial.print(",");
    Serial.print(mover._y);
    Serial.print(",");
    Serial.print(mover._x + mover._xSpeed + 3);
    Serial.print(",");
    Serial.println(mover._y + mover._ySpeed + 3);
  }
}

/**
 * Call this from setup() to initialize the OLED screen
 */
void initializeOledAndShowStartupScreen(){
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  _display.clearDisplay();

  /*_display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK);
  _display.setCursor(0, 0);
  _display.println("Screen initialized!");
  _display.display();
  delay(500);
  _display.clearDisplay();*/
}

/**
 * Call this every frame to calculate frame rate
 */
void calcFrameRate() {
  unsigned long elapsedTime = millis() - _fpsStartTimeStamp;
  _frameCount++;
  if (elapsedTime > 1000) {
    _fps = _frameCount / (elapsedTime / 1000.0);
    _fpsStartTimeStamp = millis();
    _frameCount = 0;
  }
}

/**
 * Draws the status bar at top of screen with fps
 */
void drawStatusBar() {
  // Draw frame count
  int16_t x1, y1;
  uint16_t w, h;
  _display.getTextBounds("XX.XX fps", 0, 0, &x1, &y1, &w, &h);
  _display.setCursor(_display.width() - w, 0);
  _display.print(_fps);
  _display.print(" fps");
}
