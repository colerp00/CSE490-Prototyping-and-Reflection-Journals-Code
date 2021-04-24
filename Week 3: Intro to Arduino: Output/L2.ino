const int LED_OUTPUT_PIN = 3;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_OUTPUT_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_OUTPUT_PIN, HIGH);
  delay(200);
  digitalWrite(LED_OUTPUT_PIN,LOW);
  delay(200);
  digitalWrite(LED_OUTPUT_PIN, HIGH);
  delay(200);
  digitalWrite(LED_OUTPUT_PIN,LOW);
  delay(200);
  digitalWrite(LED_OUTPUT_PIN, HIGH);
  delay(750);
  digitalWrite(LED_OUTPUT_PIN,LOW);
  delay(750);
}
