/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 6 Mar, 2023
    Update On: 22 May, 2023
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
 //========Nextion Library and Object========//
#include "Nextion.h"  //Nextion libraries
//Declare the object
//page id:0, component id:3, component name:j0
NexGauge guageldr = NexGauge(6, 5, "guageldr");
NexText textldr = NexText(5, 3, "textldr");

// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
};
 //========Nextion Library and Object========//

//===================LDR=====================//
#define INPUT_ldr 34
//===================LDR=====================//

//=================Simple Timer==============//
#include <SimpleTimer.h>
SimpleTimer timer;
//=================Simple Timer==============//

//===============Setup Function==============//
void setup() {
  Serial.begin(9600);
  nexInit();  //Inital the tft

  timer.setInterval(3000L, sendldrtoNextion);
}
//===============Setup Function==============//

//================LDRtoNextion===============//
void sendldrtoNextion() {
  float ldr_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_ldr);
  //Serial.print("Law LDR data 1:");
  //Serial.println(sensor_analog1);
  ldr_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));

  Serial.print("LDR Percentage 1 = ");
  Serial.print(ldr_percentage1);
  Serial.print("%\n\n");
  //delay(1000);

  String command1 = "textldr.txt=\"" + String(ldr_percentage1) + "\"";
  Serial.print(command1);
  endNextionCommand();

  String command2 = "textldr.txt=\"" + String(ldr_percentage1) + "\"";
  Serial.print(command2);
  endNextionCommand();

  int val = map(ldr_percentage1, 0, 100, 0, 222);
  Serial.print("guageldr.val=");  //Send the object tag
  Serial.print(val);              //Send the value
  endNextionCommand();
}
//================LDRtoNextion===============//

//=============endNextionCommand=============//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//=============endNextionCommand=============//

//===============Loop function===============//
void loop() {
  //When push/pop event occured execute component in touch event list
  nexLoop(nex_listen_list);
  timer.run();
}
//===============Loop function===============//
