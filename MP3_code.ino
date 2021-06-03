#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ParallaxJoystick.hpp>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const int JOYSTICK_UPDOWN_PIN = A1;
const int JOYSTICK_LEFTRIGHT_PIN = A0;

const uint16_t MAX_ANALOG_VAL = 1023;
const enum JoystickYDirection JOYSTICK_Y_DIR = RIGHT;

// Analog joystick
ParallaxJoystick _analogJoystick(JOYSTICK_UPDOWN_PIN, JOYSTICK_LEFTRIGHT_PIN, MAX_ANALOG_VAL, JOYSTICK_Y_DIR);

const int ATTACK_BUTTON_PIN = 12;
const int BLOCK_BUTTON_PIN = 13;

String _lastSerialSent = "";

// If false, only sends new data when the new analog value does not
// equal the last analog value. If true, always sends the data
boolean _alwaysSendData = false;

void setup() {
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  pinMode(ATTACK_BUTTON_PIN, INPUT);
  pinMode(BLOCK_BUTTON_PIN, INPUT);

  _display.setTextSize(1);      // Normal 1:1 pixel scale
  _display.setTextColor(SSD1306_WHITE); // Draw white text
  _display.setCursor(0, 0);     // Start at top-left corner
}

void loop() {
  _display.clearDisplay();

  String rcvdSerialData = Serial.readStringUntil('\n');
  
  String toSerial = playerMovement() + "," + playerAction();
  if (_alwaysSendData || toSerial != _lastSerialSent) {
    Serial.println(toSerial);
    _lastSerialSent = toSerial;
  }
  _display.setCursor(0, 0);
  //if (Serial.available() > 0) {
    toSerial = "#" + rcvdSerialData;
    Serial.println(toSerial);
    _display.println("RECEIVED:\n");
    _display.println(rcvdSerialData);
  //}
  _display.display();
  /*if (rcvdSerialData.substring(0,1).equals("#")) {
    _display.println("RECEIVED:\n");
    _display.println(rcvdSerialData);
  }*/

  /*String rcvdSerialData = Serial.readStringUntil('\n');
  _display.setCursor(0, 0);
  _display.setTextSize(1);
  _display.println("RECEIVED:\n");
  //_display.setTextSize(3);
  _display.println(rcvdSerialData);
  _display.display();*/
}

String playerMovement() {
  _analogJoystick.read();

  int upDownVal = _analogJoystick.getUpDownVal();
  int leftRightVal = _analogJoystick.getLeftRightVal();
  
  // Translate joystick movement into amount of pixels to move
  //int yMovementPixels = map(upDownVal, 0, MAX_ANALOG_VAL + 1, -5, 6);
  //int xMovementPixels = map(leftRightVal, 0, MAX_ANALOG_VAL + 1, -5, 6);
  float xFrac = leftRightVal / (float)MAX_ANALOG_VAL;
  float yFrac = upDownVal / (float)MAX_ANALOG_VAL;

  return String(xFrac,1) + "," + String(yFrac,1); //String(xMovementPixels) + "," + String(yMovementPixels);
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
