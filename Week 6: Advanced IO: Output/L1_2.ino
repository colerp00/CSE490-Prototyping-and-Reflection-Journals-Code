#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Ball struct
typedef struct ball_stats {
  const int _radius;
  int _x;  // x location of the ball
  int _y;  // y location of the ball
  int _xSpeed; // x speed of ball (in pixels per frame)
  int _ySpeed; // y speed of ball (in pixels per frame)
} Ball;

// Ball 1
Ball ball1 = {5, SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT / 2, 0, 0};
Ball ball2 = {10, SCREEN_WIDTH / 2 + 10, SCREEN_HEIGHT / 2, 0, 0};

// for tracking fps
float _fps = 0;
unsigned long _frameCount = 0;
unsigned long _fpsStartTimeStamp = 0;

// status bar
const boolean _drawStatusBar = true; // change to show/hide status bar

void setup() {
  Serial.begin(9600);
  
  initializeOledAndShowStartupScreen();

  // Get random speeds for both balls
  getRandomSpeed(&ball1);
  getRandomSpeed(&ball2);

  _fpsStartTimeStamp = millis();
}

void loop() {
  _display.clearDisplay();

  if(_drawStatusBar){
    drawStatusBar();
  }

  calcFrameRate();

  updateBall(&ball1);
  updateBall(&ball2);
  //checkBalls(&ball1, &ball2);

  printStats(ball1, "Ball 1");
  printStats(ball2, "Ball 2");
  
  drawBall(ball1, false);
  drawBall(ball2, true);
  
  // Render buffer to screen
  _display.display();
}

void printStats(Ball b, char* ballNum) {
  Serial.print(ballNum);
  Serial.print(": ");
  Serial.print("(");
  Serial.print(b._x);
  Serial.print(",");
  Serial.print(b._y);
  Serial.print("),(");
  Serial.print(b._xSpeed);
  Serial.print(",");
  Serial.print(b._ySpeed);
  Serial.println(")");
}

void getRandomSpeed(Ball* b) {
  // Gets a random long between min and max - 1
  // https://www.arduino.cc/reference/en/language/functions/random-numbers/random/
  b->_xSpeed = random(1, 4);
  b->_ySpeed = random(1, 4);
}

void drawBall(Ball b, bool fill) {
  if(fill) {
    // Draw and fill in circle
    _display.fillCircle(b._x, b._y, b._radius, SSD1306_WHITE);
  } else {
    // Draw circle outline
    _display.drawCircle(b._x, b._y, b._radius, SSD1306_WHITE);
  }
}

void updateBall(Ball* b) {
  // Update ball based on speed location
  b->_x += b->_xSpeed;
  b->_y += b->_ySpeed;
  checkTopBottom(b);
  checkLeftRight(b);
}

/*void checkBalls(Ball* b1, Ball* b2) {
  int xDist = abs(b1->_x - b2->_x);
  int yDist = abs(b1->_y - b2->_y);
  int totalDistSqrd = xDist^2 + yDist^2;
  int minDistSqrd = (b1->_radius + b2->_radius)^2;
  // Check for bouncing off other ball
  if(totalDistSqrd < minDistSqrd) {
    getNewSpeed(b1, b1->_xSpeed, b1->_ySpeed, xDist, yDist);
    getNewSpeed(b2, b2->_xSpeed, b2->_ySpeed, xDist, yDist);
  }
}*/


/*void getNewSpeed(Ball* b, int xSpeed, int ySpeed, int xDist, int yDist) {
  int newYSpeed = ySpeed*(xDist^2) - ySpeed*(yDist^2) - 2*xSpeed*yDist*xDist;
  int newXSpeed = xSpeed*(xDist^2) - xSpeed*(yDist^2) + 2*ySpeed*yDist*xDist;
  b->_xSpeed = newXSpeed/newYSpeed;
  b->_ySpeed = newYSpeed/newXSpeed;
}*/

void checkTopBottom(Ball* b) {
  // Check for bouncing off left or right side of screen
  if(b->_x - b->_radius <= 0 || b->_x + b->_radius >= _display.width()){
    b->_xSpeed = b->_xSpeed * -1; // reverse x direction
  }
}

void checkLeftRight(Ball* b) {
  // Check for bouncing on floor or ceiling
  if(b->_y - b->_radius <= 0 || b->_y + b->_radius >= _display.height()){
    b->_ySpeed = b->_ySpeed * -1; // reverse y direction
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

  _display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK);
  _display.setCursor(0, 0);
  _display.println("Screen initialized!");
  _display.display();
  delay(500);
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
