#define relay1        32
#define relay2        33
//==========Setup==========//
void setup()
{
  Serial.begin(9600);
  delay(100);
//Set Mode
  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
//Set default status
  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,HIGH);
}
//==========Setup==========//

//=====Loop Function======// 
void loop() {
  digitalWrite(relay1,HIGH);
  delay(1000);
  digitalWrite(relay1,LOW);
  delay(1000);

  digitalWrite(relay2,HIGH);
  delay(1000);
  digitalWrite(relay2,LOW);
  delay(1000);


  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,HIGH);
  delay(1000);

  digitalWrite(relay1,LOW);
  digitalWrite(relay2,LOW);
  delay(1000);
}
//=====Loop Function======// 
