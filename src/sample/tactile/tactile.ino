const int BUTTON_PIN_1 = 9;
const int BUTTON_PIN_2 = 10;
const int BUTTON_PIN_3 = 11;

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);
}

void loop() {
  int btn1 = digitalRead(BUTTON_PIN_1);
  int btn2 = digitalRead(BUTTON_PIN_2);
  int btn3 = digitalRead(BUTTON_PIN_3);

  // NOTE: LOW = pressed, HIGH = not pressed (because of INPUT_PULLUP)

  if (btn1 == LOW) {
    Serial.println("Button 1 Pressed");
  }

  if (btn2 == LOW) {
    Serial.println("Button 2 Pressed");
  }

  if (btn3 == LOW) {
    Serial.println("Button 3 Pressed");
  }

  delay(200); // simple debounce
}