const int VIBROMOTOR_OUTPUT_PIN = 3;
const int ANALOG_INPUT_PIN = A0;

void setup() {
  pinMode(VIBROMOTOR_OUTPUT_PIN, OUTPUT);
  //pinMode(ANALOG_INPUT_PIN, INPUT);
}

void loop() {
  int analogIn = analogRead(ANALOG_INPUT_PIN);
  int mapping = map(analogIn, 0, 1023, 0, 255);
  analogWrite(VIBROMOTOR_OUTPUT_PIN, mapping);
  analogWrite(LED_BUILTIN, mapping);
  delay(100);
}
