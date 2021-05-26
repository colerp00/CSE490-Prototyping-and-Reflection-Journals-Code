/**
 * Receives a string off of the serial port and prints it out
 * on the OLED display.
 * 
 * By Jon E. Froehlich
 * @jonfroehlich
 * http://makeabilitylab.io
 * 
 */

// #include <SPI.h> // Comment out when using i2c
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int DELAY_MS = 5;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int RED_OUTPUT_PIN = 9;
const int GRN_OUTPUT_PIN = 10;
const int BLU_OUTPUT_PIN = 11;

void setup() {
  Serial.begin(9600);

  pinMode(RED_OUTPUT_PIN, OUTPUT);
  pinMode(GRN_OUTPUT_PIN, OUTPUT);
  pinMode(BLU_OUTPUT_PIN, OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  _display.clearDisplay();
  _display.setTextSize(1);      // Normal 1:1 pixel scale
  _display.setTextColor(SSD1306_WHITE); // Draw white text
  _display.setCursor(0, 0);     // Start at top-left corner
  _display.print("Waiting to receive\ndata from serial...");
  _display.display();
}

void loop() {
  // Check to see if there is any incoming serial data
  if(Serial.available() > 0){
    // If we're here, then serial data has been received
    // Read data off the serial port until we get to the endline delimeter ('\n')
    // Store all of this data into a string
    String rcvdSerialData = Serial.readStringUntil('\n');

    // Parse the comma separated string
    int startIndex = 0;
    int endIndex = rcvdSerialData.indexOf(','); // find first index of comma in the string
    if(endIndex != -1) {
      // Parse out the red value
      String strRedVal = rcvdSerialData.substring(startIndex, endIndex);
      int redVal = strRedVal.toInt();

      // Parse out the green value
      startIndex = endIndex + 1;
      endIndex = rcvdSerialData.indexOf(',', startIndex);
      String strGrnVal = rcvdSerialData.substring(startIndex, endIndex);
      int grnVal = strGrnVal.toInt();

      // Parse out the blue value
      startIndex = endIndex + 1;
      endIndex = rcvdSerialData.length();
      String strBluVal = rcvdSerialData.substring(startIndex, endIndex);
      int bluVal = strBluVal.toInt();

      redVal = constrain(redVal, 0, 255);
      grnVal = constrain(grnVal, 0, 255);
      bluVal = constrain(bluVal, 0, 255);

      analogWrite(RED_OUTPUT_PIN, redVal);
      analogWrite(GRN_OUTPUT_PIN, grnVal);
      analogWrite(BLU_OUTPUT_PIN, bluVal);
    }
    
    
    _display.clearDisplay();
    int16_t x1, y1;
    uint16_t textWidth, textHeight;

    _display.setTextSize(2);  

    // Measure one line of text     
    _display.getTextBounds("Test", 0, 0, &x1, &y1, &textWidth, &textHeight);
    int oneLineOfTextHeight = textHeight;

    // Measure the height of the received data
    _display.getTextBounds(rcvdSerialData, 0, 0, &x1, &y1, &textWidth, &textHeight);

    // Automatically set font size and centering and display received text
    if(textHeight <= oneLineOfTextHeight){
      // Write small amounts of text to center of screen
      uint16_t yText = _display.height() / 2 - textHeight / 2;
      uint16_t xText = _display.width() / 2 - textWidth / 2; 
      _display.setCursor(xText, yText);
      _display.print(rcvdSerialData);
      _display.display();
    }else{
      // Once text gets too large, write it smaller and start at 0,0
      _display.setTextSize(1);  
      _display.setCursor(0, 0);
      _display.print(rcvdSerialData);
      _display.display();
    }
    
    uint16_t yText = _display.height() / 2 - textHeight / 2;
    uint16_t xText = _display.width() / 2 - textWidth / 2;
    
    // Echo the data back on serial (for debugging purposes)
    // This is not necessary but helpful. Then the webpage can
    // display this debug output (if necessary)
    Serial.print("Arduino received: '");
    Serial.print(rcvdSerialData);
    Serial.println("'");
  }

  //delay(DELAY_MS);
}
