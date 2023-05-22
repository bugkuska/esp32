/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 6 Mar, 2023
    Update On : 22 May,2023
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
    Hoo ya, have fun....
 *************************************************************************************************/
//=========Nextion Library and Object========//
#include "Nextion.h"  //Nextion libraries
//Declare the object
//page id:0, component id:3, component name:j0
//NexDSButton bt0 = NexDSButton(0, 4, "bt0");
NexButton bt0 = NexButton(2, 2, "bt0");
NexButton bt1 = NexButton(2, 3, "bt1");
// Declare variable global
bool statusbt0 = false;
bool statusbt1 = false;
// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
  &bt0,
  &bt1,
  NULL
};
//=========Nextion Library and Object========//

//=========Define IO connect to relay========//
#define sw1 18
#define sw2 19
//=========Define IO connect to relay========//

//==============ON,OFF Relay1================//
void bt0PushCallback(void *ptr) {
  if (statusbt0 == false) {
    digitalWrite(sw1, LOW);
    statusbt0 = true;
  } else if (statusbt0 == true) {
    digitalWrite(sw1, HIGH);
    statusbt0 = false;
  }
}
//==============ON,OFF Relay1================//

//==============ON,OFF Relay2================//
void bt1PushCallback(void *ptr) {
  if (statusbt1 == false) {
    digitalWrite(sw2, LOW);
    statusbt1 = true;
  } else {
    digitalWrite(sw2, HIGH);
    statusbt1 = false;
  }
}
//==============ON,OFF Relay2================//

//===============Setup function==============//
void setup() {
  Serial.begin(9600);
  nexInit();  //Inital the tft

  // Register the push/pop event callback function
  bt0.attachPush(bt0PushCallback, &bt0);
  bt1.attachPush(bt1PushCallback, &bt1);

  // Set IO pinMode for relay
  pinMode(sw1, OUTPUT);
  pinMode(sw2, OUTPUT);

  //Set default relay status at boot
  digitalWrite(sw1, HIGH);
  digitalWrite(sw2, HIGH);
}
//===============Setup function==============//

//===============Loop function===============//
void loop() {
  //When push/pop event occured execute component in touch event list
  nexLoop(nex_listen_list);
}
//===============Loop function===============//
