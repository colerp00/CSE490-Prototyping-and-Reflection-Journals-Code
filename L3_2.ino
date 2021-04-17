const int LED_OUTPUT_PIN = 3;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_OUTPUT_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_OUTPUT_PIN, HIGH);
  Serial.println("Pin 3 is HIGH (5V)");
  delay(200);
  
  digitalWrite(LED_OUTPUT_PIN,LOW);
  Serial.println("Pin 3 is LOW (0V)");
  delay(200);
  
  digitalWrite(LED_OUTPUT_PIN, HIGH);
  Serial.println("Pin 3 is HIGH (5V)");
  delay(200);
  
  digitalWrite(LED_OUTPUT_PIN,LOW);
  Serial.println("Pin 3 is LOW (0V)");
  delay(200);
  
  digitalWrite(LED_OUTPUT_PIN, HIGH);
  Serial.println("Pin 3 is HIGH (5V)");
  delay(750);
  
  digitalWrite(LED_OUTPUT_PIN,LOW);
  Serial.println("Pin 3 is LOW (0V)");
  delay(750);
}
