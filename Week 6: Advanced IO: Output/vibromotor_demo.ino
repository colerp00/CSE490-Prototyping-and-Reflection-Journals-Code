const int VIBROMOTOR_OUTPUT_PIN = 3;

void setup() {
  pinMode(VIBROMOTOR_OUTPUT_PIN, OUTPUT);
}

void loop() {
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
