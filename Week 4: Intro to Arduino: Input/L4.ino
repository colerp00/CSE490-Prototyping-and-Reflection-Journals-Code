const int INPUT_PIN = A0;
const int OUTPUT_PIN = 3;

void setup() {
  pinMode(OUTPUT_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int potVal = analogRead(INPUT_PIN);
  analogWrite(OUTPUT_PIN, potVal / 4);
  Serial.print(potVal);
  Serial.print(", ");
  Serial.println(potVal / 4);
  delay(50);
}
