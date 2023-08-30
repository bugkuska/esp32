#define relay1        25
#define relay2        33
#define relay3        32 
#define relay4        23
//==========Setup==========//
void setup()
{
  Serial.begin(9600);
  delay(100);

  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(relay3,OUTPUT);
  pinMode(relay4,OUTPUT);

  digitalWrite(relay1,LOW);
  digitalWrite(relay2,LOW);
  digitalWrite(relay3,LOW);
  digitalWrite(relay4,LOW);
}
//==========Setup==========//



//=====Loop Function======// 
void loop() {
  digitalWrite(relay1,LOW);
  delay(1000);
  digitalWrite(relay1,HIGH);
  delay(1000);

  digitalWrite(relay2,LOW);
  delay(1000);
  digitalWrite(relay2,HIGH);
  delay(1000);

  digitalWrite(relay3,LOW);
  delay(1000);
  digitalWrite(relay3,HIGH);
  delay(1000);

  digitalWrite(relay4,LOW);
  delay(1000);
  digitalWrite(relay4,HIGH);
  delay(1000);

  digitalWrite(relay1,LOW);
  digitalWrite(relay2,LOW);
  digitalWrite(relay3,LOW);
  digitalWrite(relay4,LOW);
  delay(1000);

  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,HIGH);
  digitalWrite(relay3,HIGH);
  digitalWrite(relay4,HIGH);
  delay(1000);
}
//=====Loop Function======// 
