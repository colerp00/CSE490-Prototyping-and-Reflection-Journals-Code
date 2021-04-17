const int LED_OUTPUT_PIN = 3;
const int MAX_ANALOG_OUT = 255; // the max analog output on the Uno is 255
const int DELAY_MS = 5;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_OUTPUT_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // fade on slowly
  for(int i = 0; i <= MAX_ANALOG_OUT; i += 1){
    analogWrite(LED_OUTPUT_PIN, i);
    delay(DELAY_MS);
  }

  // fade off quickly
  for(int i = MAX_ANALOG_OUT; i >= 0; i -= 3){
    analogWrite(LED_OUTPUT_PIN, i);
    delay(DELAY_MS);
  }

  // fade on quickly
  for(int i = 0; i <= MAX_ANALOG_OUT; i += 3){
    analogWrite(LED_OUTPUT_PIN, i);
    delay(DELAY_MS);
  }

  // fade off slowly
  for(int i = MAX_ANALOG_OUT; i >= 0; i -= 1){
    analogWrite(LED_OUTPUT_PIN, i);
    delay(DELAY_MS);
  }
}
