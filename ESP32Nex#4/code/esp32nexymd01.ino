/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 21 Mar, 2023
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
  *Note : ระหว่าง upload source-code ให้ถอดสาย RX0 ออกจากบอร์ด ESP32 พอ upload source-code ค่อยต่อเข้าไปใหม่
  2. Download and install Modbus-Master, Simple-Timer
    Hoo ya, have fun....
 *************************************************************************************************/
 //=========Nextion Library and Object==========//
#include "Nextion.h"  //Nextion libraries
// Nextion Objects, ada 5 tombol
// (page id, component id, component name)
//Page 3 Dual State button
NexButton bt0 = NexButton(3, 2, "bt0");
NexButton bt1 = NexButton(3, 3, "bt1");
//Page4 Text XY-MD02
NexText textemp1 = NexText(4, 2, "textemp1");
NexText texhumi = NexText(4, 3, "texhumi");
//Page 5 Progress BarXY-MD02
NexText textemp2 = NexText(5, 2, "textemp2");
NexText texhumi2 = NexText(5, 3, "texhumi2");
NexProgressBar pgtemp1 = NexProgressBar(5, 4, "pgtemp1");
NexProgressBar pghumi1 = NexProgressBar(5, 5, "pghumi1");
//Page 6 Guage XY-MD02
NexGauge guagetemp1 = NexGauge(6, 2, "guagetemp1");
NexGauge guagehumi1 = NexGauge(6, 3, "guagehumi1");
//Page 7 Guage LDR
NexGauge guageldr = NexGauge(7, 2, "guageldr");
NexText textldr = NexText(7, 3, "textldr");

// Declare variable global
bool statusbt0 = false;
bool statusbt1 = false;

// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
  &bt0,
  &bt1,
  NULL
};
 //=========Nextion Library and Object==========//

//==========Define IO connect to relay==========//
#define sw1 18
#define sw2 19
//==========Define IO connect to relay==========//

//=======================LDR====================//
#define INPUT_ldr 34
//=======================LDR====================//

//=================Modbus-Master================//
#include <ModbusMaster.h>
#define RXD2 16
#define TXD2 17
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 XY-MD02
float humi1 = 0.0f;
float temp1 = 0.0f;
//=================Modbus-Master================//

//=================SimpleTimer==================//
#include <SimpleTimer.h>
SimpleTimer timer;
//=================SimpleTimer==================//

//================ON,OFF Relay1=================//
void bt0PushCallback(void *ptr) {
  if (statusbt0 == false) {
    digitalWrite(sw1, LOW);
    statusbt0 = true;
  } else if (statusbt0 == true) {
    digitalWrite(sw1, HIGH);
    statusbt0 = false;
  }
}
//================ON,OFF Relay1=================//

//=================ON,OFF Relay2================//
void bt1PushCallback(void *ptr) {
  if (statusbt1 == false) {
    digitalWrite(sw2, LOW);
    statusbt1 = true;
  } else {
    digitalWrite(sw2, HIGH);
    statusbt1 = false;
  }
}
//=================ON,OFF Relay2================//

//=================Setup Function===============//
void setup() {
  Serial.begin(9600);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node1.begin(1, Serial2);
  //Initialize Nextion Library
  nexInit();

  // Register the push/pop event callback function
  bt0.attachPush(bt0PushCallback, &bt0);
  bt1.attachPush(bt1PushCallback, &bt1);
  // Set IO pinMode for relay
  pinMode(sw1, OUTPUT);
  pinMode(sw2, OUTPUT);
  //Set default relay status at boot
  digitalWrite(sw1, HIGH);
  digitalWrite(sw2, HIGH);

  timer.setInterval(5000L, xymdtoNex);  //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(3000L, sendldrtoNex);
}
//=================Setup Function===============//

//==============XY-MD02 to Nextion==============//
void xymdtoNex() {
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

  //Page 3 Text XY-MD02
  String command = "textemp1.txt=\"" + String(temp1) + "\"";
  Serial.print(command);
  endNextionCommand();

  String command1 = "textemp1.txt=\"" + String(temp1) + "\"";
  Serial.print(command1);
  endNextionCommand();

  String command2 = "texhumi.txt=\"" + String(humi1) + "\"";
  Serial.print(command2);
  endNextionCommand();

  //Page 4 Progress Bar XY-MD02
  String command3 = "textemp2.txt=\"" + String(temp1) + "\"";
  Serial.print(command3);
  endNextionCommand();

  String command4 = "textemp2.txt=\"" + String(temp1) + "\"";
  Serial.print(command4);
  endNextionCommand();

  String command5 = "texhumi2.txt=\"" + String(humi1) + "\"";
  Serial.print(command5);
  endNextionCommand();

  pgtemp1.setValue(temp1);
  Serial.print("pgtemp1.val=");
  Serial.print(temp1);
  endNextionCommand();

  pghumi1.setValue(humi1);
  Serial.print("pghumi1.val=");
  Serial.print(humi1);
  endNextionCommand();

  //Page 5 Guage XY-MD02
  int val1 = map(temp1, 0, 100, 0, 223);
  Serial.print("guagetemp1.val=");  //Send the object tag
  Serial.print(val1);               //Send the value
  endNextionCommand();

  int val2 = map(humi1, 0, 100, 0, 223);
  Serial.print("guagehumi1.val=");  //Send the object tag
  Serial.print(val2);               //Send the value
  endNextionCommand();
}
//==============XY-MD02 to Nextion==============//

//=================LDRtoNextion=================//
void sendldrtoNex() {
  float ldr_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_ldr);
  ldr_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));

  Serial.print("LDR Percentage 1 = ");
  Serial.print(ldr_percentage1);
  Serial.print("%\n\n");

//Page 6 Guage LDR
  String command3 = "textldr.txt=\"" + String(ldr_percentage1) + "\"";
  Serial.print(command3);
  endNextionCommand();

  String command4 = "textldr.txt=\"" + String(ldr_percentage1) + "\"";
  Serial.print(command4);
  endNextionCommand();

  int val = map(ldr_percentage1, 0, 100, 0, 222);
  Serial.print("guageldr.val=");  //Send the object tag
  Serial.print(val);              //Send the value
  endNextionCommand();
}
//=================LDRtoNextion=================//

//===============endNextionCommand==============//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//===============endNextionCommand==============//

//==================Loop Function===============//
void loop() {
  //When push/pop event occured execute component in touch event list
  nexLoop(nex_listen_list);
  timer.run();
}
//==================Loop Function===============//
