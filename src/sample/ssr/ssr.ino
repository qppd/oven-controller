#define SSR1_PIN 2
#define SSR2_PIN 3

void setup() {
  pinMode(SSR1_PIN, OUTPUT);
  pinMode(SSR2_PIN, OUTPUT);

  // Start OFF
  digitalWrite(SSR1_PIN, LOW);
  digitalWrite(SSR2_PIN, LOW);
}

void loop() {
  // Turn BOTH ON
  digitalWrite(SSR1_PIN, HIGH);
  digitalWrite(SSR2_PIN, HIGH);
  delay(2000); // 2 seconds ON

  // Turn BOTH OFF
  digitalWrite(SSR1_PIN, LOW);
  digitalWrite(SSR2_PIN, LOW);
  delay(2000); // 2 seconds OFF
}