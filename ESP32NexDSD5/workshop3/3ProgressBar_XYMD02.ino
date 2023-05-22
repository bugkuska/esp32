/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 6 Mar, 2023
    Update On : 22 May, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
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
    TX >> RX0, ถ้าแก้ไขไฟล์ NexConfig.h ใน Library ITEADLIB_Arduino_Nextion สามารถเปลี่ยนมาใช้ RX0 ได้
  *Note : ระหว่าง upload source-code ให้ถอดสาย RX2 ออกจากบอร์ด ESP32 พอ upload source-code ค่อยต่อเข้าไปใหม่
  2. Download and install Modbus-Master, Simple-Timer
    Hoo ya, have fun....
 *************************************************************************************************/
//========Nextion Library and Object==========//
#include "Nextion.h"  //Nextion libraries
// (page id, component id, component name)
NexText textemp2 = NexText(4, 2, "textemp2");
NexText texhumi2 = NexText(4, 3, "texhumi2");
NexProgressBar pgtemp1 = NexProgressBar(4, 4, "pgtemp1");
NexProgressBar pghumi1 = NexProgressBar(4, 5, "pghumi1");
// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
};
//========Nextion Library and Object==========//

//===============Modbus-Master================//
#include <ModbusMaster.h>
#define RXD2 16
#define TXD2 17
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 WTR10-E
float humi1 = 0.0f;
float temp1 = 0.0f;
//===============Modbus-Master================//

//===============SimpleTimer==================//
#include <SimpleTimer.h>
SimpleTimer timer;
//===============SimpleTimer==================//

//===============Setup Function===============//
void setup() {
  Serial.begin(9600);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node1.begin(1, Serial2);
  //Initialize Nextion Library
  nexInit();

  timer.setInterval(5000L, wtr2Nextion);  //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
}
//===============Setup Function===============//

//=============XY_MD02toNextion===============//
void wtr2Nextion() {
 uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0)/10.0f);
  float humi1 = (node1.getResponseBuffer(1)/10.0f);

  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2); //Function 04, Read 2 registers starting at 2)
  if (result1 == node1.ku8MBSuccess) {
    Serial.print("Temp1: ");
    Serial.println(node1.getResponseBuffer(0)/10.0f);
    Serial.print("Humi1: ");
    Serial.println(node1.getResponseBuffer(1)/10.0f);
  }
  
  String command = "textemp2.txt=\"" + String(temp1) + "\"";
  Serial.print(command);
  endNextionCommand();

  String command1 = "textemp2.txt=\"" + String(temp1) + "\"";
  Serial.print(command1);
  endNextionCommand();

  String command2 = "texhumi2.txt=\"" + String(humi1) + "\"";
  Serial.print(command2);
  endNextionCommand();

  pgtemp1.setValue(temp1);
  Serial.print("pgtemp1.val=");
  Serial.print(temp1);
  endNextionCommand();

  pghumi1.setValue(humi1);
  Serial.print("pghumi1.val=");
  Serial.print(humi1);
  endNextionCommand();
}
//=============XY_MD02toNextion===============//

//=============endNextionCommand==============//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//=============endNextionCommand==============//

//================Loop Function===============//
void loop() {
  //When push/pop event occured execute component in touch event list
  nexLoop(nex_listen_list);
  timer.run();
}
//================Loop Function===============//
