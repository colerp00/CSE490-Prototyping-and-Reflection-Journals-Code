class Blinker{

  private:
    const int _pin;           // output pin
    unsigned long _interval;  // blink interval in ms

    int _state;                     // current state (either HIGH OR LOW)
    unsigned long _lastToggledTs;   // last state toggle in ms
    int _intervalToggle;            // toggle for interval grow (1) or shrink (-1)

  public:

    // Constructor
    Blinker(int pin, unsigned long blinkInterval) :
      _pin(pin)
    {
      _interval = blinkInterval;
      _state = LOW;
      _lastToggledTs = 0;
      _intervalToggle = 1;
      pinMode(_pin, OUTPUT);
    }

    /**
     * Calculates whether to toggle output state based on the set interval
     * Call this function once per loop()
     */ 
    void update() {
      unsigned long currentTsMs = millis();
      
      if (currentTsMs - _lastToggledTs >= _interval) {
        _lastToggledTs = currentTsMs;
        _state = !_state;
        digitalWrite(_pin, _state);
        if (_intervalToggle == 1) {
          _interval = _interval - 50;
        } else {
          _interval = _interval + 50;
        }

        if (_interval <= 0 || _interval >= 1000) {
          _intervalToggle = -_intervalToggle;
        }
      }
    }
};

Blinker _led1Blinker(2, 200);  // specify pin and blink interval (200ms)
Blinker _led2Blinker(5, 400);  // specify pin and blink interval (400ms)
Blinker _led3Blinker(9, 800);  // specify pin and blink interval (800ms)

// The setup function runs once when you press reset or power the board
void setup() {
  // empty 
}

// The loop function runs over and over again forever
void loop() {
  _led1Blinker.update();
  _led2Blinker.update();
  _led3Blinker.update();
}
