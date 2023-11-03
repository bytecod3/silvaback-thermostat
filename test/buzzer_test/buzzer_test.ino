
#define BUZZER 2
void setup() {
  // put your setup code here, to run once:
  pinMode(BUZZER, OUTPUT);

  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
  delay(100);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
