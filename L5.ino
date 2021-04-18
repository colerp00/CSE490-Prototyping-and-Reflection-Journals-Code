const int LED1_OUTPUT_PIN = 3;
const int LED2_OUTPUT_PIN = 4;
const int INTERVAL_MS = 1000;

unsigned long _lastTimeBlinkedMS = 0;
int _ledState = LOW;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED1_OUTPUT_PIN, OUTPUT);
  pinMode(LED2_OUTPUT_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

  unsigned long thisTimeBlinkedMS = millis();
  
  if(thisTimeBlinkedMS - _lastTimeBlinkedMS >= INTERVAL_MS) {
    _ledState = _ledState == HIGH ? LOW : HIGH;
    _lastTimeBlinkedMS = thisTimeBlinkedMS;
    digitalWrite(LED1_OUTPUT_PIN, _ledState);
    digitalWrite(LED2_OUTPUT_PIN, _ledState);
  }
}
