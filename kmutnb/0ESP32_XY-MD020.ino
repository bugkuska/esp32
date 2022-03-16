#include <SimpleTimer.h>
SimpleTimer timer;
#include <ModbusMaster.h>
#define MAX485_DE       18    //DE //Direction
#define MAX485_RE_NEG   19    //RE //Direction
#define RX2             16    //RO,RX
#define TX2             17    //DI,TX

//===========Modbus Object=========//
ModbusMaster node1; //XY-MD02
//===========Modbus Object=========//

//======pre&post transmission======//
void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

//======pre&post transmission======//

//=========Setup Function==========//
void setup()
{
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  // use Serial (port 0); initialize debug serial comunication
  Serial.begin(9600);    
   // use Serial (port 1); initialize Modbus communication baud rate
   // Ref https://www.arduino.cc/reference/en/language/functions/communication/serial/begin/
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); //RX2=16,RO ,TX2=17, DI
    // communicate with Modbus slave ID 1 over Serial (port 1)
  node1.begin(1, Serial2);
  node1.preTransmission(preTransmission);
  node1.postTransmission(postTransmission);
  
  timer.setInterval(5000L,xymd02); //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
}

//=========Setup Function==========//

//===========XY-MD02 Sensor=======//
void xymd02()//Function ในการอ่านค่าจาก Sensor 
{
  uint8_t result1; 
  float temp1 = (node1.getResponseBuffer(0)/10.0f);
  float humi1 = (node1.getResponseBuffer(1)/10.0f);

  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2); //Function 04, Read 2 registers starting at 2)
  if (result1 == node1.ku8MBSuccess)
  {
    Serial.print("Temp: ");
    Serial.println(node1.getResponseBuffer(0)/10.0f);
    Serial.print("Humi: ");
    Serial.println(node1.getResponseBuffer(1)/10.0f);
  }
  delay(1000);
}

//===========XY-MD02 Sensor=======//

//==========Loop Function=========//
void loop()
{
timer.run();
}
//==========Loop Function=========//
