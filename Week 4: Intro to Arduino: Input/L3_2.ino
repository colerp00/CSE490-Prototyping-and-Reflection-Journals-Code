#define KEY_C 262  // 261.6256 Hz (middle C)
#define KEY_D 294  // 293.6648 Hz
#define KEY_E 330  // 329.6276 Hz
#define KEY_F 350  // 349.2282 Hz
#define KEY_G 392  // 391.9954 Hz

const int INPUT_BUTTON_C_PIN = 2;
const int INPUT_BUTTON_D_PIN = 4;
const int INPUT_BUTTON_E_PIN = 7;
const int INPUT_BUTTON_F_PIN = 8;
const int INPUT_BUTTON_G_PIN = 12;

const int OUTPUT_LED_WHT_PIN = 3;
const int OUTPUT_LED_RED_PIN = 11;
const int OUTPUT_LED_YLW_PIN = 6;
const int OUTPUT_LED_GRN_PIN = 9;
const int OUTPUT_LED_BLU_PIN = 10;

const int OUTPUT_PIEZO_PIN = 5;
const int OUTPUT_LED_PIN = LED_BUILTIN;

const int MAX_ANALOG_OUT = 255;

const int TIME_ON_EACH_FADE_VAL_MS = 30;

class Button {
  private:
    const int _pinIn;
    const int _pinOut;
    const int _key;

    int _ledCurBrightness;
    unsigned long _brightnessUpdateTS;
    
    int _savedRawState;
    int _savedDebouncedState;
    unsigned long _stateChangeTS;

    const int _debouncedWindow = 40;
    const int _fadeAmount = 5;
  
    public:

    // Constructor
    Button(int in, int out, int key) :
      _pinIn(in),
      _pinOut(out),
      _key(key)
    {
      _savedRawState = LOW;
      _savedDebouncedState = LOW;
      _stateChangeTS = 0;
      _ledCurBrightness = 0;
      _brightnessUpdateTS = 0;
      pinMode(_pinIn, INPUT_PULLUP);
      pinMode(_pinOut, OUTPUT);
    }

    void debounce(unsigned long currentTimestampMs) {
      int buttonVal = digitalRead(_pinIn);
      
      if(buttonVal != _savedRawState) {
        _stateChangeTS = currentTimestampMs;
      }
      
      if(millis() - _stateChangeTS >= _debouncedWindow) {
        
        if(buttonVal == HIGH && buttonVal != _savedDebouncedState) {
          analogWrite(_pinOut, MAX_ANALOG_OUT);
          _ledCurBrightness = MAX_ANALOG_OUT;
          noTone(OUTPUT_PIEZO_PIN);
        }
    
        _savedDebouncedState = buttonVal;
      } else {
        tone(OUTPUT_PIEZO_PIN, _key);
      }
      
      _savedRawState = buttonVal;
    }

    void fadeLED(unsigned long timestamp) {
      if(timestamp - _brightnessUpdateTS >= TIME_ON_EACH_FADE_VAL_MS && 
         _ledCurBrightness >= 0) {
        analogWrite(_pinOut, _ledCurBrightness);
        _brightnessUpdateTS = timestamp;
        _ledCurBrightness = _ledCurBrightness - _fadeAmount;
      }
    }
};

Button _whiteLEDButton(INPUT_BUTTON_C_PIN, OUTPUT_LED_WHT_PIN, KEY_C);
Button _redLEDButton(INPUT_BUTTON_D_PIN, OUTPUT_LED_RED_PIN, KEY_D);
Button _yellowLEDButton(INPUT_BUTTON_E_PIN, OUTPUT_LED_YLW_PIN, KEY_E);
Button _greenLEDButton(INPUT_BUTTON_F_PIN, OUTPUT_LED_GRN_PIN, KEY_F);
Button _blueLEDButton(INPUT_BUTTON_G_PIN, OUTPUT_LED_BLU_PIN, KEY_G);

unsigned long _whiteFadeValueUpdatedTimestampMs = 0;
unsigned long _redFadeValueUpdatedTimestampMs = 0;
unsigned long _yellowFadeValueUpdatedTimestampMs = 0;
unsigned long _greenFadeValueUpdatedTimestampMs = 0;
unsigned long _blueFadeValueUpdatedTimestampMs = 0;

void setup() {
  pinMode(OUTPUT_PIEZO_PIN, OUTPUT);

  //Serial.begin(9600);
}

void loop() {
  unsigned long currentTimestampMs = millis();

  _whiteLEDButton.fadeLED(currentTimestampMs);
  _redLEDButton.fadeLED(currentTimestampMs);
  _yellowLEDButton.fadeLED(currentTimestampMs);
  _greenLEDButton.fadeLED(currentTimestampMs);
  _blueLEDButton.fadeLED(currentTimestampMs);

  _whiteLEDButton.debounce(currentTimestampMs);
  _redLEDButton.debounce(currentTimestampMs);
  _yellowLEDButton.debounce(currentTimestampMs);
  _greenLEDButton.debounce(currentTimestampMs);
  _blueLEDButton.debounce(currentTimestampMs);
}
