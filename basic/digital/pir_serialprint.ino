#define ledpin 2
#define pirpin 19
int val = 0;
void setup() {
  pinMode(ledpin, OUTPUT);
  pinMode(pirpin, INPUT);

  digitalWrite(ledpin, HIGH);
  Serial.begin(9600);
}

void loop() {
  val = digitalRead(pirpin);
  Serial.print("val = ");
  Serial.println(val);
  if (val == 1) {
    digitalWrite(ledpin, HIGH);
    Serial.print("Motion detect");
  }
  else {
    digitalWrite(ledpin, LOW);
    Serial.print("No things");
  }
  delay(100);
}
