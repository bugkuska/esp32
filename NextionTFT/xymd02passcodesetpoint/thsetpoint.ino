/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 25 Feb, 2023
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
    TX >> RX2, ถ้าแก้ไขไฟล์ NexConfig.h ใน Library ITEADLIB_Arduino_Nextion สามารถเปลี่ยนมาใช้ RX0 ได้
  *Note : ระหว่าง upload source-code ให้ถอดสาย RX2 ออกจากบอร์ด ESP32 พอ upload source-code ค่อยต่อเข้าไปใหม่
  2. Download and install Modbus-Master, Blynk and Simple-Timer
    Hoo ya, have fun....
 *************************************************************************************************/
//==============Modbus-Master==============//
#include <ModbusMaster.h>
#define RXD2 16
#define TXD2 17
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 XY-MD02
float temp1 = 0.0f;  //Map to Blynk virtual pin V4
float humi1 = 0.0f;  //Map to Blynk virtual pin V3
//==============Modbus-Master==============//

//==============SimpleTimer===============//
#include <SimpleTimer.h>
SimpleTimer timer;
//==============SimpleTimer===============//

//============Define GPIO=================//
#define relay1 18
#define relay2 19
//============Define GPIO=================//

uint32_t valuesetpoint;  //value setpoint for temperature
uint32_t valuesetpointh; //value setpoint for humidity

//======Nextion Library and Object=======//
#include "Nextion.h"  //Nextion libraries
// (page id, component id, component name)
//Temperature
NexText tstate1 = NexText(2, 3, "tstate1");
NexText textemp1 = NexText(2, 4, "textemp1");
NexNumber nsetpoint = NexNumber(2, 2, "nsetpoint");
NexNumber nvalsetpoint = NexNumber(2, 1, "nvalsetpoint");
NexGauge guagetemp1 = NexGauge(2, 5, "guagetemp1");
//Humidity
NexText tstate2 = NexText(3, 7, "tstate2");
NexText texthumi1 = NexText(3, 5, "texthumi1");
NexNumber nsetpointh = NexNumber(3, 6, "nsetpointh");
NexNumber nvalsetpointh = NexNumber(3, 9, "nvalsetpointh");
NexGauge guagehumi1 = NexGauge(3, 8, "guagehumi1");
//======Nextion Library and Object=======//

//============Setup Function=============//
void setup() {
  //Initialize Nextion Library
  nexInit();
  Serial.begin(9600);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  //RX2=io16, TX2=io17
  node1.begin(1, Serial2);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);

  timer.setInterval(10000L, xymd);
}
//============Setup Function=============//

//=============XY-MD02toNex==============//
void xymd() {
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float humi1 = (node1.getResponseBuffer(1) / 10.0f);

  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2);  //Function 04, Read 2 registers starting at 2)
  if (result1 == node1.ku8MBSuccess) {
    Serial.print("Temp1: ");
    Serial.println(node1.getResponseBuffer(0) / 10.0f);
    Serial.print("Humi1: ");
    Serial.println(node1.getResponseBuffer(1) / 10.0f);
  }
//Temperature
  String command = "textemp1.txt=\"" + String(temp1) + "\"";
  Serial.print(command);
  endNextionCommand();

  String command1 = "textemp1.txt=\"" + String(temp1) + "\"";
  Serial.print(command1);
  endNextionCommand();
  
  int val1 = map(temp1, 0, 100, 0, 225);
  Serial.print("guagetemp1.val=");  //Send the object tag
  Serial.print(val1);               //Send the value
  endNextionCommand();

  nsetpoint.getValue(&valuesetpoint);
  endNextionCommand();

  nvalsetpoint.setValue(valuesetpoint);
  endNextionCommand();

  nsetpoint.getValue(&valuesetpoint);
  endNextionCommand();

  nvalsetpoint.setValue(valuesetpoint);
  endNextionCommand();

  if (temp1 > valuesetpoint) {
    digitalWrite(relay1, LOW);
    tstate1.setText("ON");
  } else {
    digitalWrite(relay1, HIGH);
    tstate1.setText("OFF");
  }

//Humidity
String command2 = "texthumi1.txt=\"" + String(humi1) + "\"";
  Serial.print(command2);
  endNextionCommand();

  String command3 = "texthumi1.txt=\"" + String(humi1) + "\"";
  Serial.print(command3);
  endNextionCommand();
  
  int val2 = map(humi1, 0, 100, 0, 225);
  Serial.print("guagehumi1.val=");  //Send the object tag
  Serial.print(val2);               //Send the value
  endNextionCommand();

  nsetpointh.getValue(&valuesetpointh);
  endNextionCommand();

  nvalsetpointh.setValue(valuesetpointh);
  endNextionCommand();

  nsetpointh.getValue(&valuesetpointh);
  endNextionCommand();

  nvalsetpointh.setValue(valuesetpointh);
  endNextionCommand();

  if (humi1 < valuesetpointh) {
    digitalWrite(relay2, LOW);
    tstate2.setText("ON");
  } else {
    digitalWrite(relay2, HIGH);
    tstate2.setText("OFF");
  }
}
//=============XY-MD02toNex==============//

//===========endNextionCommand===========//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//===========endNextionCommand===========//

//=============Loop Function=============//
void loop() {
  timer.run();
}
//=============Loop Function=============//
