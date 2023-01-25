/*************************************************************************************************
    ESP32+PZEM016
    Created By: Sompoch Tongnamtiang
    Created On: 25 JAN, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
    Tel: 081-411-1142
    Line ID : bugkuska
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1.Download and install ITEADLIB_Arduino_Nextion library:
    https://github.com/itead/ITEADLIB_Arduino_Nextion
  *  *********************************************************************************************
    Wiring Nextion to ESP32
    5V >> 5V
    GND >> GND
    RX >> TX0
    TX >> RX0
  *Note : ระหว่าง upload source-code ให้ถอดสาย RX0 ออกจากบอร์ด ESP32 พอ upload source-code ค่อยต่อเข้าไปใหม่
    Hoo ya, have fun....
 *************************************************************************************************/

//==================Modbus================//
#include <ModbusMaster.h>
///////// PIN /////////
#define RX2 16  //RX
#define TX2 17  //TX
// instantiate ModbusMaster object
ModbusMaster node;
static uint8_t pzemSlaveAddr = 0x01;
int Reset_Energy = 0;
//==================Modbus================//
#include "Nextion.h"  //Nextion libraries
//==============SimpleTimer===============//
#include <SimpleTimer.h>
SimpleTimer timer;
//==============SimpleTimer===============//
//page id:x, component id:x, component name:x
NexText txtvolt = NexText(1, 2, "txtvolt");
NexText txtcurrent = NexText(1, 3, "txtcurrent");
NexText txthz = NexText(1, 4, "txthz");
NexText txtpower = NexText(1, 5, "txtpower");
NexText txtenergy = NexText(1, 6, "txtenergy");
//==============Setup Function============//

// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
  &txtvolt,
  &txtcurrent,
  &txthz,
  &txtpower,
  &txtenergy,
  NULL
};

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);  //RX2=16,TX2=17
  node.begin(pzemSlaveAddr, Serial2);

  nexInit();

  timer.setInterval(5000L, Pzem016);
}
//==============Setup Function============//

/*
BLYNK_WRITE(V7)
{
  if (param.asInt() == 1)
  {
    Reset_Energy = Reset_Energy + 1;
    Serial.println(Reset_Energy);
    if (Reset_Energy > 2) {
      Serial.println("Resetting Energy");
      uint16_t u16CRC = 0xFFFF;
      static uint8_t resetCommand = 0x42;
      uint8_t slaveAddr = pzemSlaveAddr;
      u16CRC = crc16_update(u16CRC, slaveAddr);
      u16CRC = crc16_update(u16CRC, resetCommand);
      //preTransmission1();
      Serial2.write(slaveAddr);
      Serial2.write(resetCommand);
      Serial2.write(lowByte(u16CRC));
      Serial2.write(highByte(u16CRC));
      delay(10);
      //postTransmission1();
      delay(100);
    }
  }
}
*/
void resetEnergy(uint8_t slaveAddr)  //Reset the slave's energy counter
{
  uint16_t u16CRC = 0xFFFF;
  static uint8_t resetCommand = 0x42;
  u16CRC = crc16_update(u16CRC, slaveAddr);
  u16CRC = crc16_update(u16CRC, resetCommand);
  Serial.println("Resetting Energy");
  //preTransmission();
  Serial2.write(slaveAddr);
  Serial2.write(resetCommand);
  Serial2.write(lowByte(u16CRC));
  Serial2.write(highByte(u16CRC));
  delay(10);
  //postTransmission();
  delay(100);
  while (Serial2.available()) {  // Prints the response from the Pzem, do something with it if you like
    Serial.print(char(Serial2.read()), HEX);
    Serial.print(" ");
  }
}

//============Read Pzem016================//
void Pzem016() {
  uint32_t tempdouble = 0x00000000;

  float PZEMVoltage = node.getResponseBuffer(0x0000) / 10.0;  //get the 16bit value for the voltage, divide it by 10 and cast in the float variable

  tempdouble = (node.getResponseBuffer(0x0002) << 16) + node.getResponseBuffer(0x0001);  // Get the 2 16bits registers and combine them to an unsigned 32bit
  float PZEMCurrent = tempdouble / 1000.00;                                              // Divide the unsigned 32bit by 1000 and put in the current float variable

  tempdouble = (node.getResponseBuffer(0x0004) << 16) + node.getResponseBuffer(0x0003);
  float PZEMPower = tempdouble / 10.0;

  tempdouble = (node.getResponseBuffer(0x0006) << 16) + node.getResponseBuffer(0x0005);
  float PZEMEnergy = tempdouble / 1000.00;

  float PZEMHz = node.getResponseBuffer(0x0007) / 10.0;

  float PZEMPower_factor = node.getResponseBuffer(0x0008) / 100.00;

  uint8_t result;

  result = node.readInputRegisters(0x0000, 9);  //read the 9 registers of the PZEM-014 / 016

  if (result == node.ku8MBSuccess) {
    /*uint32_t tempdouble = 0x00000000;

    float PZEMVoltage = node.getResponseBuffer(0x0000) / 10.0;  //get the 16bit value for the voltage, divide it by 10 and cast in the float variable

    tempdouble = (node.getResponseBuffer(0x0002) << 16) + node.getResponseBuffer(0x0001);  // Get the 2 16bits registers and combine them to an unsigned 32bit
    float PZEMCurrent = tempdouble / 1000.00;                                              // Divide the unsigned 32bit by 1000 and put in the current float variable

    tempdouble = (node.getResponseBuffer(0x0004) << 16) + node.getResponseBuffer(0x0003);
    float PZEMPower = tempdouble / 10.0;

    tempdouble = (node.getResponseBuffer(0x0006) << 16) + node.getResponseBuffer(0x0005);
    float PZEMEnergy = tempdouble / 1000.00;

    float PZEMHz = node.getResponseBuffer(0x0007) / 10.0;
    float PZEMPower_factor = node.getResponseBuffer(0x0008) / 100.00;
    */
    /*
    Serial.print(PZEMVoltage, 1);  // Print Voltage with 1 decimal
    Serial.print("V   ");

    Serial.print(PZEMHz, 1);
    Serial.print("Hz   ");

    Serial.print(PZEMCurrent, 1);
    Serial.print("A   ");

    Serial.print(PZEMPower, 1);
    Serial.print("W  ");

    Serial.print(PZEMPower_factor, 2);
    Serial.print("pf   ");

    Serial.print(PZEMEnergy, 1);
    Serial.print("Wh  ");
    Serial.println();
    delay(1000);
*/
  }
  String command1 = "txtvolt.txt=\"" + String(PZEMVoltage) + "\"";
  Serial.print(command1);
  //endNextionCommand();
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  String command2 = "txtcurrent.txt=\"" + String(PZEMCurrent) + "\"";
  Serial.print(command2);
  //endNextionCommand();
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  String command3 = "txthz.txt=\"" + String(PZEMHz) + "\"";
  Serial.print(command3);
  //endNextionCommand();
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  String command4 = "txtpower.txt=\"" + String(PZEMPower) + "\"";
  Serial.print(command4);
  //endNextionCommand();
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  String command5 = "txtenergy.txt=\"" + String(PZEMEnergy) + "\"";
  Serial.print(command5);
  //endNextionCommand();
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  //}
  // delay(1000);
}
//============Read Pzem016================//

//========endNextionCommand=========//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//========endNextionCommand=========//

//==============Loop Function============//
void loop() {
  nexLoop(nex_listen_list);
  timer.run();
}
//==============Loop Function============//
