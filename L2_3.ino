#define KEY_C 262  // 261.6256 Hz (middle C)
#define KEY_D 294  // 293.6648 Hz
#define KEY_E 330  // 329.6276 Hz
#define KEY_F 350  // 349.2282 Hz
#define KEY_G 392  // 391.9954 Hz

const int INPUT_BUTTON_C_PIN = 2;
const int INPUT_BUTTON_D_PIN = 5;
const int INPUT_BUTTON_E_PIN = 4;
const int INPUT_BUTTON_F_PIN = 7;
const int INPUT_BUTTON_G_PIN = 8;

const int OUTPUT_LED_WHT_PIN = 3;
const int OUTPUT_LED_RED_PIN = 9;
const int OUTPUT_LED_YLW_PIN = 10;
const int OUTPUT_LED_GRN_PIN = 11;
const int OUTPUT_LED_BLU_PIN = 6;

const int OUTPUT_PIEZO_PIN = 13;
const int OUTPUT_LED_PIN = LED_BUILTIN;

const int MAX_ANALOG_OUT = 255;

const int TIME_ON_EACH_FADE_VAL_MS = 30;

const boolean _buttonsAreActiveLow = true;

int _fadeAmount = 5;
int _curBrightnessWhite = 0;
int _curBrightnessRed = 0;
int _curBrightnessYellow = 0;
int _curBrightnessGreen = 0;
int _curBrightnessBlue = 0;

unsigned long _whiteFadeValueUpdatedTimestampMs = 0;
unsigned long _redFadeValueUpdatedTimestampMs = 0;
unsigned long _yellowFadeValueUpdatedTimestampMs = 0;
unsigned long _greenFadeValueUpdatedTimestampMs = 0;
unsigned long _blueFadeValueUpdatedTimestampMs = 0;

void setup() {
  pinMode(INPUT_BUTTON_C_PIN, INPUT_PULLUP);
  pinMode(INPUT_BUTTON_D_PIN, INPUT_PULLUP);
  pinMode(INPUT_BUTTON_E_PIN, INPUT_PULLUP);
  pinMode(INPUT_BUTTON_F_PIN, INPUT_PULLUP);
  pinMode(INPUT_BUTTON_G_PIN, INPUT_PULLUP);
  pinMode(OUTPUT_PIEZO_PIN, OUTPUT);
  pinMode(OUTPUT_LED_WHT_PIN, OUTPUT);
  pinMode(OUTPUT_LED_RED_PIN, OUTPUT);
  pinMode(OUTPUT_LED_YLW_PIN, OUTPUT);
  pinMode(OUTPUT_LED_GRN_PIN, OUTPUT);
  pinMode(OUTPUT_LED_BLU_PIN, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  unsigned long currentTimestampMs = millis();
  fadeLEDs(currentTimestampMs);
  
  if(isButtonPressed(INPUT_BUTTON_C_PIN)){
    tone(OUTPUT_PIEZO_PIN, KEY_C);
    analogWrite(OUTPUT_LED_WHT_PIN, MAX_ANALOG_OUT);
    _curBrightnessWhite = MAX_ANALOG_OUT;
    Serial.println("White button pressed!");
  }else if(isButtonPressed(INPUT_BUTTON_D_PIN)){
    tone(OUTPUT_PIEZO_PIN, KEY_D);
    analogWrite(OUTPUT_LED_RED_PIN, MAX_ANALOG_OUT);
    _curBrightnessRed = MAX_ANALOG_OUT;
    Serial.println("Red button pressed!");
  }else if(isButtonPressed(INPUT_BUTTON_E_PIN)){
    tone(OUTPUT_PIEZO_PIN, KEY_E);
    analogWrite(OUTPUT_LED_YLW_PIN, MAX_ANALOG_OUT);
    _curBrightnessYellow = MAX_ANALOG_OUT;
    Serial.println("Yellow button pressed!");
  }else if(isButtonPressed(INPUT_BUTTON_F_PIN)){
    tone(OUTPUT_PIEZO_PIN, KEY_F);
    analogWrite(OUTPUT_LED_GRN_PIN, MAX_ANALOG_OUT);
    _curBrightnessGreen = MAX_ANALOG_OUT;
    Serial.println("Green button pressed!");
  }else if(isButtonPressed(INPUT_BUTTON_G_PIN)){
    tone(OUTPUT_PIEZO_PIN, KEY_G);
    analogWrite(OUTPUT_LED_BLU_PIN, MAX_ANALOG_OUT);
    _curBrightnessBlue = MAX_ANALOG_OUT;
    Serial.println("Blue button pressed!");
  }else{
    noTone(OUTPUT_PIEZO_PIN); // turn off the waveform
    digitalWrite(OUTPUT_LED_PIN, LOW);
  }
}

boolean isButtonPressed(int btnPin){
  int btnVal = digitalRead(btnPin);
  if(_buttonsAreActiveLow && btnVal == LOW){
    // button is hooked up with pull-up resistor
    // and is in a pressed state
    digitalWrite(OUTPUT_LED_PIN, HIGH);
    return true;
  }else if(!_buttonsAreActiveLow && btnVal == HIGH){
    // button is hooked up with a pull-down resistor
    // and is in a pressed state
    digitalWrite(OUTPUT_LED_PIN, HIGH);
    return true;
  }

  // button is not pressed
  return false;
}

void fadeLEDs(unsigned long timestamp) {
  Serial.println("Fading LEDs!");
  Serial.println(_curBrightnessWhite);
  Serial.println(_curBrightnessRed);
  Serial.println(_curBrightnessYellow);
  Serial.println(_curBrightnessGreen);
  Serial.println(_curBrightnessBlue);
  
  if(timestamp - _whiteFadeValueUpdatedTimestampMs >= TIME_ON_EACH_FADE_VAL_MS && 
        _curBrightnessWhite >= 0) {
    analogWrite(OUTPUT_LED_WHT_PIN, _curBrightnessWhite);
    _whiteFadeValueUpdatedTimestampMs = timestamp;
    _curBrightnessWhite = _curBrightnessWhite - _fadeAmount;
    Serial.print("Fading white button:");
    Serial.println(_curBrightnessWhite);
  }
  if(timestamp - _redFadeValueUpdatedTimestampMs >= TIME_ON_EACH_FADE_VAL_MS && 
        _curBrightnessRed >= 0) {
    analogWrite(OUTPUT_LED_RED_PIN, _curBrightnessRed);
    _redFadeValueUpdatedTimestampMs = timestamp;
    _curBrightnessRed = _curBrightnessRed - _fadeAmount;
    Serial.print("Fading red button:");
    Serial.println(_curBrightnessRed);
  }
  if(timestamp - _yellowFadeValueUpdatedTimestampMs >= TIME_ON_EACH_FADE_VAL_MS && 
        _curBrightnessYellow >= 0) {
    analogWrite(OUTPUT_LED_YLW_PIN, _curBrightnessYellow);
    _yellowFadeValueUpdatedTimestampMs = timestamp;
    _curBrightnessYellow = _curBrightnessYellow - _fadeAmount;
    Serial.print("Fading yellow button:");
    Serial.println(_curBrightnessYellow);
  }
  if(timestamp - _greenFadeValueUpdatedTimestampMs >= TIME_ON_EACH_FADE_VAL_MS && 
        _curBrightnessGreen >= 0) {
    analogWrite(OUTPUT_LED_GRN_PIN, _curBrightnessGreen);
    _greenFadeValueUpdatedTimestampMs = timestamp;
    _curBrightnessGreen = _curBrightnessGreen - _fadeAmount;
    Serial.print("Fading green button:");
    Serial.println(_curBrightnessGreen);
  }
  if(timestamp - _blueFadeValueUpdatedTimestampMs >= TIME_ON_EACH_FADE_VAL_MS && 
        _curBrightnessBlue >= 0) {
    analogWrite(OUTPUT_LED_BLU_PIN, _curBrightnessBlue);
    _blueFadeValueUpdatedTimestampMs = timestamp;
    _curBrightnessBlue = _curBrightnessBlue - _fadeAmount;
    Serial.print("Fading blue button:");
    Serial.println(_curBrightnessBlue);
  }
}
