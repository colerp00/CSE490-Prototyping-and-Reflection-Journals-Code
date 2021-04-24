const int INPUT_BUTTON_RED_PIN = 2;
const int INPUT_BUTTON_GRN_PIN = 5;
const int INPUT_BUTTON_BLU_PIN = 8;
const int OUTPUT_RED_LED_PIN = 3;
const int OUTPUT_GRN_LED_PIN = 6;
const int OUTPUT_BLU_LED_PIN = 9;

void setup() {
  pinMode(INPUT_BUTTON_RED_PIN, INPUT);
  pinMode(OUTPUT_RED_LED_PIN, OUTPUT);

  pinMode(INPUT_BUTTON_GRN_PIN, INPUT);
  pinMode(OUTPUT_GRN_LED_PIN, OUTPUT);

  pinMode(INPUT_BUTTON_BLU_PIN, INPUT);
  pinMode(OUTPUT_BLU_LED_PIN, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  int redButtonVal = digitalRead(INPUT_BUTTON_RED_PIN);
  int greenButtonVal = digitalRead(INPUT_BUTTON_GRN_PIN);
  int blueButtonVal = digitalRead(INPUT_BUTTON_BLU_PIN);

  if(redButtonVal == HIGH) {
    Serial.println("Red Button pressed!");
  } else {
    Serial.println("Red Button is NOT pressed.");
  }

  if(greenButtonVal == LOW) {
    Serial.println("Green Button pressed!");
  } else {
    Serial.println("Green Button is NOT pressed.");
  }

  if(blueButtonVal == HIGH) {
    Serial.println("Blue Button pressed!");
  } else {
    Serial.println("Blue Button is NOT pressed.");
  }
  
  digitalWrite(OUTPUT_RED_LED_PIN, redButtonVal);
  digitalWrite(OUTPUT_GRN_LED_PIN, greenButtonVal);
  digitalWrite(OUTPUT_BLU_LED_PIN, blueButtonVal);

  delay(5);
}
