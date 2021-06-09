#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ParallaxJoystick.hpp>
#include <future>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//const int GPIO_PINS[] = {26, 25, 4, 5, 18, 19, 16, 17, 21, 13, 12, 27, 33, 15, 32, 14, 22, 23};

// For some reason, A12 is not defined. So, using raw literal 13 instead
// ADC#1 includes A2, A3, A4, A7, A9
// ADC#2 includes A0, A1, A5, A6, A8, A10, A11, and A12
//const int ANALOG_INPUT_PINS[] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, 13};


const int JOYSTICK_UPDOWN_PIN = A1;
const int JOYSTICK_LEFTRIGHT_PIN = A2;

const uint16_t MAX_ANALOG_VAL = 4095;
const enum JoystickYDirection JOYSTICK_Y_DIR = RIGHT;

// Analog joystick
ParallaxJoystick _analogJoystick(JOYSTICK_UPDOWN_PIN, JOYSTICK_LEFTRIGHT_PIN, MAX_ANALOG_VAL, JOYSTICK_Y_DIR);

const int ATTACK_BUTTON_PIN = 33;
const int BLOCK_BUTTON_PIN = 27;

String _lastSerialSent = "";

// If false, only sends new data when the new analog value does not
// equal the last analog value. If true, always sends the data
boolean _alwaysSendData = false;

struct room {
  String id;
  uint8_t x;
  uint8_t y;
};

struct room _visitedAreas[9];
uint8_t _numAreasVisited = 1;
String _curArea;

const uint8_t ROOM_SIDE = 16;
const uint8_t ROOM_HALF = ROOM_SIDE / 2;
unsigned long _lastShownPTs;
bool _showingP;
const uint16_t SHOW_P_INTERVAL = 1000;


void setup() {
  Serial.begin(230400);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //_visitedAreas = new String[6];
  struct room room1;
  room1.id = "room 0-1";
  room1.x = SCREEN_WIDTH / 2;
  room1.y = ROOM_HALF;
  _visitedAreas[0] = room1;
  _curArea = room1.id;

  _lastShownPTs = millis();

  pinMode(ATTACK_BUTTON_PIN, INPUT);
  pinMode(BLOCK_BUTTON_PIN, INPUT);

  _display.setTextSize(1);      // Normal 1:1 pixel scale
  _display.setTextColor(SSD1306_WHITE); // Draw white text
  _display.setCursor(0, 0);     // Start at top-left corner

  //String toSerial = "Joystick max analog val: " + String(_analogJoystick.getMaxAnalogValue()) + " Center: " + String(_analogJoystick.getCenter());
  //Serial.println(toSerial);
  /*String test1 = "cur:room 0-2,true;64,22";
  readSerialTest(test1);
  String test3 = "cur:room 1-1,true;78,22";
  readSerialTest(test3);
  String test4 = "cur:room 1-2,true;92,22";
  readSerialTest(test4);
  String test5 = "cur:room 2-1,true;50,22";
  readSerialTest(test5);
  String test6 = "cur:room 2-2,true;36,22";
  readSerialTest(test6);

  String test2 = "cur:room 0-2,false;";
  readSerialTest(test2);*/
}

void loop() {
  _display.clearDisplay();

  //String rcvdSerialData = Serial.readStringUntil('\n');
  std::future<bool> futRead = std::async(readSerial);
  std::future<bool> futWrite = std::async(writeSerial);

  bool red = futRead.get();
  _display.setCursor(0, 0);
  /*for (int i = 0; i < _numAreasVisited; i++) {
    _display.println(_visitedAreas[i].id);
  }
  _display.setCursor(SCREEN_WIDTH / 2,0);
  _display.println(_curArea);*/
  
  /*if (Serial.available() > 0) {
    toSerial = "#" + rcvdSerialData;
    Serial.println(toSerial);
    _display.println("RECEIVED:\n");
    _display.println(rcvdSerialData);
  }*/
  //String rcvdSerialData = fut.get();
  drawRooms();
  bool written = futWrite.get();
  _display.display();
}

bool readSerial() {
  if (Serial.available() > 0) {
    String rcvdSerialData = Serial.readStringUntil('\n');
    if (rcvdSerialData.startsWith("cur")) {
      uint8_t comInd = rcvdSerialData.indexOf(",");
      uint8_t colInd = rcvdSerialData.indexOf(":");
      uint8_t semiInd = rcvdSerialData.indexOf(";");
      String room = rcvdSerialData.substring(colInd+1,comInd);
      if (rcvdSerialData.substring(comInd+1,comInd+5) == "true") {
        struct room newRoom;
        newRoom.id = room;
        String coords = rcvdSerialData.substring(semiInd+1);
        uint8_t com2Ind = coords.indexOf(",");
        newRoom.x = coords.substring(0,com2Ind).toInt();
        newRoom.y = coords.substring(com2Ind+1).toInt();
        _visitedAreas[_numAreasVisited] = newRoom;
        _numAreasVisited++;
      }
      _curArea = room;
      return true;
    }
  }
  return false;
}

/*bool readSerialTest(String serial) {
  //String rcvdSerialData = Serial.readStringUntil('\n');
  Serial.println("\n");
  Serial.println("serial = " + serial);
    if (serial.startsWith("cur")) {
      uint8_t comInd = serial.indexOf(",");
      uint8_t colInd = serial.indexOf(":");
      uint8_t semiInd = serial.indexOf(";");
      String room = serial.substring(colInd+1,comInd);
      Serial.println("true = " + serial.substring(comInd+1,comInd+5));
      if (serial.substring(comInd+1,comInd+5) == "true") {
        struct room newRoom;
        newRoom.id = room;
        String coords = serial.substring(semiInd+1);
        uint8_t com2Ind = coords.indexOf(",");
        Serial.println("coords = " + coords);
        Serial.println("x = " + coords.substring(0,com2Ind));
        Serial.println("y = " + coords.substring(com2Ind+1));
        newRoom.x = (coords.substring(0,com2Ind)).toInt();
        newRoom.y = (coords.substring(com2Ind+1)).toInt();
        Serial.println(newRoom.id);
        Serial.println(newRoom.x);
        Serial.println(newRoom.y);
        _visitedAreas[_numAreasVisited] = newRoom;
        _numAreasVisited++;
      }
      _curArea = room;
      return true;
    }
    return false;
}*/

bool writeSerial() {
  String toSerial = playerMovement() + "," + playerAction();
  if (_alwaysSendData || toSerial != _lastSerialSent) {
    Serial.println(toSerial);
    _lastSerialSent = toSerial;
  }
  return true;
}

bool drawRooms() {
  int x = 0, y = 0;
  for (int i = 0; i < _numAreasVisited; i++) {
    _display.fillRect(_visitedAreas[i].x - ROOM_HALF,
                      _visitedAreas[i].y - ROOM_HALF,
                      ROOM_SIDE,
                      ROOM_SIDE,
                      SSD1306_WHITE);
    _display.fillRect(_visitedAreas[i].x - ROOM_HALF + 2,
                      _visitedAreas[i].y - ROOM_HALF + 2,
                      ROOM_SIDE - 4,
                      ROOM_SIDE - 4,
                      SSD1306_BLACK);
    if (_visitedAreas[i].id == _curArea) {
      x = _visitedAreas[i].x;
      y = _visitedAreas[i].y;
    }
  }

  _display.fillRect((SCREEN_WIDTH / 2) - 2,
                    ROOM_SIDE - 2,
                    4, 2, SSD1306_BLACK);
  _display.fillRect((SCREEN_WIDTH / 2) - ROOM_SIDE - ROOM_HALF,
                    2 * ROOM_SIDE - ROOM_HALF - 4,
                    3 * ROOM_SIDE, 4, SSD1306_BLACK);

  if (_showingP) {
    _display.setCursor(x - ROOM_HALF + 4, y - ROOM_HALF + 4);
    _display.println("P");
  }

  unsigned long curTs = millis();
  if (curTs - _lastShownPTs >= SHOW_P_INTERVAL) {
    _showingP = !_showingP;
    _lastShownPTs = curTs;
  }
}

String playerMovement() {
  _analogJoystick.read();

  int upDownVal = _analogJoystick.getUpDownVal();
  int leftRightVal = _analogJoystick.getLeftRightVal();
  
  // Translate joystick movement into amount of pixels to move
  int yMovementPixels = map(upDownVal, 0, MAX_ANALOG_VAL + 1, -3, 4);
  int xMovementPixels = map(leftRightVal, 0, MAX_ANALOG_VAL + 1, -3, 3);
  //float xFrac = leftRightVal / (float)MAX_ANALOG_VAL;
  //float yFrac = upDownVal / (float)MAX_ANALOG_VAL;
  if (yMovementPixels == 4) {
    yMovementPixels--;
  }
  
  //return String(xFrac,1) + "," + String(yFrac,1); //String(xMovementPixels) + "," + String(yMovementPixels);
  //return String(upDownVal) + "," + String(leftRightVal);
  return String(xMovementPixels) + "," + String(yMovementPixels);
}

String playerAction() {
  if (digitalRead(BLOCK_BUTTON_PIN) == HIGH) {
    return "block";
  } else if (digitalRead(ATTACK_BUTTON_PIN) == HIGH) {
    return "attack";
  } else {
    return "";
  }
}
