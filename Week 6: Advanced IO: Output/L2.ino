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

const int VIBROMOTOR_OUTPUT_PIN = 9;
const int INPUT_BUTTON_PIN = 7;

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
const boolean _drawStatusBar = true; // change to show/hide status bar
const int DELAY_LOOP_MS = 5;

typedef struct controlled_circle {
  int _radius;
  int _xSpeed;
  int _ySpeed;
  int _x;
  int _y;
} Circle;

typedef struct hidden_circle {
  int _radius;
  int _x;
  int _y;
  int _maxXDist;
  int _maxYDist;
} Target;

const int _targetRadius = 8;

Circle controlled = {5, 0, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};

Target target; /*= {_targetRadius,
                 random(_targetRadius, SCREEN_WIDTH - _targetRadius),
                 random(_targetRadius, SCREEN_HEIGHT - _targetRadius),
                 0, 0};*/

// for flashMotor()
int _motorState = -1; // 1 = on, -1 = not on
unsigned long _lastTS = 0;
const int VIBE_INTERVAL_MS = 75;

////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);

  pinMode(VIBROMOTOR_OUTPUT_PIN, OUTPUT);
  pinMode(INPUT_BUTTON_PIN, INPUT);

  playerWins();
  
  initializeOledAndShowStartupScreen();

  initializeNewTarget();
  
  drawControlled();
}

////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  //digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
  Serial.print(target._x);
  Serial.print(",");
  Serial.println(target._y);
  
  _display.clearDisplay();

  if(_drawStatusBar){
    drawStatusBar();
  }

  calcFrameRate();

  // Read button to determine if player has won
  int buttonVal = digitalRead(INPUT_BUTTON_PIN);
  
  // Read analog joystick to control player ball
  _analogJoystick.read();
  int upDownVal = _analogJoystick.getUpDownVal();
  int leftRightVal = _analogJoystick.getLeftRightVal();

  // Translate joystick movement into amount of pixels to move
  int yMovementPixels = map(upDownVal, 0, _analogJoystick.getMaxAnalogValue() + 1, 10, -11);
  int xMovementPixels = map(leftRightVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -10, 11);

  controlled._xSpeed = xMovementPixels;
  controlled._ySpeed = yMovementPixels;
  
  controlled._x += controlled._xSpeed;
  controlled._y += controlled._ySpeed;

  if(DEBUG_2) {
    Serial.print(controlled._x);
    Serial.print(",");
    Serial.print(controlled._y);
    Serial.print(",");
    Serial.print(controlled._xSpeed);
    Serial.print(",");
    Serial.println(controlled._ySpeed);
  }
  
  moveBackToBounds();
  drawControlled();

  checkProximity();

  if(buttonVal == HIGH && insideTarget()) {
    drawTarget();
    playerWins();
  } else {
    _display.display();
  }
}

void playerWins() {
  _display.display();
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);

  delay(750);
  
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
  delay(300);
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
  delay(150);
  
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
  delay(100);
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
  delay(100);
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
  delay(100);
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
  delay(100);

  analogWrite(VIBROMOTOR_OUTPUT_PIN, 155);
  delay(200);
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
  delay(100);

  digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
  delay(500);
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
  delay(1000);

  initializeNewTarget();

  //_display.clearDisplay();
}

void initializeNewTarget() {
  // make new randomized target
  target = {_targetRadius,
            random(_targetRadius, SCREEN_WIDTH - _targetRadius),
            random(_targetRadius, SCREEN_HEIGHT - _targetRadius),
            0, 0};
  
  // Get the max dist between the target and the sides of the screen
  if(target._x > SCREEN_WIDTH / 2) {
    target._maxXDist = target._x;
  } else {
    target._maxXDist = SCREEN_WIDTH - target._x;
  }

  if(target._y > SCREEN_HEIGHT / 2) {
    target._maxYDist = target._y;
  } else {
    target._maxYDist = SCREEN_HEIGHT - target._y;
  }
}

bool insideTarget() {
  return controlled._x + controlled._radius <= target._x + target._radius &&
         controlled._x - controlled._radius >= target._x - target._radius &&
         controlled._y + controlled._radius <= target._y + target._radius &&
         controlled._y - controlled._radius >= target._y - target._radius;
}

void checkProximity() {
  // If the controlled circle is inside the target completely...
  if(insideTarget()) {
    // Flash vibromotor
    flashMotor();
  } else {
    // Fade vibromotor
    fadeMotor();
  } 
}

void fadeMotor() {
  int xDist = abs(controlled._x - target._x);
  int yDist = abs(controlled._y - target._y);

  int maxCombinedDist = target._maxXDist + target._maxYDist;
  int combinedDist = xDist + yDist;

  int mapping = map(combinedDist, 0, maxCombinedDist, 255, 0);

  analogWrite(VIBROMOTOR_OUTPUT_PIN, mapping);
}

void flashMotor() {
  unsigned long curTS = millis();
  if(curTS - _lastTS >= VIBE_INTERVAL_MS) {
    _motorState *= -1;
    _lastTS = curTS;
    if(_motorState == 1) {
      digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
    } else if(_motorState == -1) {
      digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
    }
  }
}

void moveBackToBounds() {
  if(controlled._x + 5 >= SCREEN_WIDTH) {
    controlled._x = SCREEN_WIDTH - 6;
  } else if(controlled._x - 5 <= 0) {
    controlled._x = 5;
  }
  if(controlled._y + 5 >= SCREEN_HEIGHT) {
    controlled._y = SCREEN_HEIGHT - 6;
  } else if(controlled._y - 5 <= 0) {
    controlled._y = 5;
  }
}

void drawTarget() {
  _display.drawCircle(target._x, target._y, target._radius, SSD1306_WHITE);
}

void drawControlled() {
  _display.fillCircle(controlled._x, controlled._y, controlled._radius, SSD1306_WHITE);
  //_display.drawLine(controlled._x, controlled._y, controlled._x + controlled._xSpeed, controlled._y + controlled._ySpeed, SSD1306_WHITE);
  if(DEBUG_1) {
    Serial.println("drawThing");
    Serial.print(controlled._x);
    Serial.print(",");
    Serial.print(controlled._y);
    Serial.print(",");
    Serial.print(controlled._x + controlled._xSpeed + 3);
    Serial.print(",");
    Serial.println(controlled._y + controlled._ySpeed + 3);
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
