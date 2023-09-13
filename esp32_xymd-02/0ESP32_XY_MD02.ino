//=============Simple Timer=============//
#include <SimpleTimer.h>
SimpleTimer timer;
//=============Simple Timer=============//
//============Modbus Master=============//
#include <ModbusMaster.h>
#define RX2 16  //RO,RX
#define TX2 17  //DI,TX
//============Modbus Master=============//
//=============Modbus Object============//
ModbusMaster node1;  //XY-MD02 Slave ID1
ModbusMaster node2;  //XY-MD02 Slave ID2
//=============Modbus Object============//

//=========IO connect to relay==========//
#define relay1 32
#define relay2 33
//=========IO connect to relay==========//

//===========Setup Function=============//
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);  //RX2=16 ,TX2=17
  node1.begin(1, Serial2);                    //XY-MD02 Slave ID1
  node2.begin(2, Serial2);                    //XY-MD02 Slave ID2

  //Set Mode
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  //Set default status
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);

  timer.setInterval(1000, xy1);
  timer.setInterval(1000, xy2);
}
//===========Setup Function=============//

//===============XY-MD02-1==============//
void xy1() {
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float humi1 = (node1.getResponseBuffer(1) / 10.0f);
  Serial.println("Get XY-MD02 Data1");
  result1 = node1.readInputRegisters(0x0001, 2);  // Read 2 registers starting at 1)
  if (result1 == node1.ku8MBSuccess) {
    Serial.print("Temp1: ");
    Serial.println(node1.getResponseBuffer(0) / 10.0f);
    Serial.print("Humi1: ");
    Serial.println(node1.getResponseBuffer(1) / 10.0f);
  }
  delay(1000);
}
//===============XY-MD02-1==============//
//===============XY-MD02-2==============//
void xy2() {
  uint8_t result2;
  float temp2 = (node2.getResponseBuffer(0) / 10.0f);
  float humi2 = (node2.getResponseBuffer(1) / 10.0f);
  Serial.println("Get XY-MD02 Data2");
  result2 = node2.readInputRegisters(0x0001, 2);  // Read 2 registers starting at 1)
  if (result2 == node2.ku8MBSuccess) {
    Serial.print("Temp2: ");
    Serial.println(node2.getResponseBuffer(0) / 10.0f);
    Serial.print("Humi2: ");
    Serial.println(node2.getResponseBuffer(1) / 10.0f);
  }
  delay(1000);
}
//===============XY-MD02-2==============//

//=============Loop Function============//
void loop() {
  timer.run();

  digitalWrite(relay1, HIGH);
  delay(1000);
  digitalWrite(relay1, LOW);
  delay(1000);

  digitalWrite(relay2, HIGH);
  delay(1000);
  digitalWrite(relay2, LOW);
  delay(1000);


  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  delay(1000);

  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  delay(1000);
}
//=============Loop Function============//
