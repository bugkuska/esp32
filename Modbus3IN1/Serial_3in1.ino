#include <SimpleTimer.h>
SimpleTimer timer;
#include <ModbusMaster.h>

///////// PIN /////////
#define MAX485_DE      18   //DE
#define MAX485_RE_NEG  19   //RE
#define RX2 16              //RO
#define TX2 17              //DI

ModbusMaster node1;
ModbusMaster node2;

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

void preTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void setup()
{
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); //RX2=16,RO ,TX2=17, DI
  node1.begin(1, Serial2);
  node1.preTransmission(preTransmission);
  node1.postTransmission(postTransmission);

    node2.begin(2, Serial2);
  node2.preTransmission(preTransmission2);
  node2.postTransmission(postTransmission2);
  
  timer.setInterval(5000L,hum_temp_light01);
}

void hum_temp_light01()
{
 uint8_t result1; 
  float humi1 = (node1.getResponseBuffer(0)/10.0f);
  float temp1 = (node1.getResponseBuffer(1)/10.0f);
  float light1 = (node1.getResponseBuffer(2));
  float light_per1;
//======node1 Slave ID 1======// 

//======node1 Slave ID 1======//   
  Serial.println("GZWS Data1");
  result1 = node1.readHoldingRegisters(0x0000, 3); // Read 3 registers starting at 1)
  if (result1 == node1.ku8MBSuccess)
  {
    Serial.print("Humi1: ");
    Serial.println(node1.getResponseBuffer(0)/10.0f);
    Serial.print("Temp1: ");
    Serial.println(node1.getResponseBuffer(1)/10.0f);
    light_per1 = (light1 = node1.getResponseBuffer(2));
    light_per1 = map(light_per1, 0,65535, 0,100);   
    Serial.print("Light1: ");
    Serial.println(light_per1);
  }
  delay(1000);
}

void loop()
{
timer.run();
}
