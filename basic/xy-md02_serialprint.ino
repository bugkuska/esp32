//============Modbus Master=============//
#include <ModbusMaster.h>
#define RX2 16  //RX
#define TX2 17  //TX
//============Modbus Master=============//
//=============Modbus Object============//
ModbusMaster node1;  //XY-MD02 Slave ID1
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
}
//===========Setup Function=============//

//=============Loop Function============//
void loop() {
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
//=============Loop Function============//
