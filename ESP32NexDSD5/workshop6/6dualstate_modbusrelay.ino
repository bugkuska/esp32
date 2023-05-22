/*************************************************************************************************
    Modbus Relay
    Created By: Sompoch Tongnamtiang
    Created On: 22 May, 2023
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
    RX >> TX0, แก้ไขไฟล์ NexConfig.h ใน Library ITEADLIB_Arduino_Nextion สามารถเปลี่ยนมาใช้ TX0 ได้ (Serial)
    TX >> RX0, แก้ไขไฟล์ NexConfig.h ใน Library ITEADLIB_Arduino_Nextion สามารถเปลี่ยนมาใช้ RX0 ได้ (Serial)
  *Note : ระหว่าง upload source-code ให้ถอดสาย RX0 ออกจากบอร์ด ESP32 พอ upload source-code ค่อยต่อเข้าไปใหม่
    Hoo ya, have fun....
 *************************************************************************************************/
#include "Nextion.h"  //Nextion libraries

//Declare the object
//page id:0, component id:3, component name:j0
//NexDSButton bt0 = NexDSButton(0, 4, "bt0");
//Modbus Relay
NexButton bt2 = NexButton(5, 5, "bt2");
NexButton bt3 = NexButton(5, 6, "bt3");
NexButton bt4 = NexButton(5, 7, "bt4");
NexButton bt5 = NexButton(5, 8, "bt5");
NexButton bt6 = NexButton(5, 9, "bt6");
NexButton bt7 = NexButton(5, 10, "bt7");
NexButton bt8 = NexButton(5, 11, "bt8");
NexButton bt9 = NexButton(5, 12, "bt9");

// Declare variable global
//Modbus Relay
bool statusbt2 = false;
bool statusbt3 = false;
bool statusbt4 = false;
bool statusbt5 = false;
bool statusbt6 = false;
bool statusbt7 = false;
bool statusbt8 = false;
bool statusbt9 = false;

// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
  &bt2,
  &bt3,
  &bt4,
  &bt5,
  &bt6,
  &bt7,
  &bt8,
  &bt9,
  NULL
};

//========Modbus-Master===========//
#include <ModbusMaster.h>
#define RXD2 16
#define TXD2 17
// instantiate ModbusMaster object
ModbusMaster node2;   //Slave ID2 Modbus RTU Relay
int8_t pool_size1;    //Pool size for Modbus Write command
//========Modbus-Master===========//

//==========Modbus Relay============//
//==========ON,OFF MRelay1==========//
void bt2PushCallback(void *ptr) {
  if (statusbt2 == false) {
    pool_size1 = node2.writeSingleRegister(0x01,0x0100);
    //relayControl_modbusRTU(2, 1, 1);
    //digitalWrite(sw1, LOW);
    statusbt2 = true;
  } else if (statusbt2 == true) {
     pool_size1 = node2.writeSingleRegister(0x01,0x0200);
    //relayControl_modbusRTU(2, 1, 0);
    //digitalWrite(sw1, HIGH);
    statusbt2 = false;
  }
}
//==========ON,OFF MRelay1==========//

//==========ON,OFF MRelay2==========//
void bt3PushCallback(void *ptr) {
  if (statusbt3 == false) {
    pool_size1 = node2.writeSingleRegister(0x02,0x0100);
    //relayControl_modbusRTU(2, 2, 1);
    //digitalWrite(sw1, LOW);
    statusbt3 = true;
  } else if (statusbt3 == true) {
    pool_size1 = node2.writeSingleRegister(0x02,0x0200);
    //relayControl_modbusRTU(2, 2, 0);
    //digitalWrite(sw1, HIGH);
    statusbt3 = false;
  }
}
//==========ON,OFF MRelay2==========//

//==========ON,OFF MRelay3==========//
void bt4PushCallback(void *ptr) {
  if (statusbt4 == false) {
     pool_size1 = node2.writeSingleRegister(0x03,0x0100);
    //relayControl_modbusRTU(2, 3, 1);
    //digitalWrite(sw1, LOW);
    statusbt4 = true;
  } else if (statusbt4 == true) {
    pool_size1 = node2.writeSingleRegister(0x03,0x0200);
    //relayControl_modbusRTU(2, 3, 0);
    //digitalWrite(sw1, HIGH);
    statusbt4 = false;
  }
}
//==========ON,OFF MRelay3==========//

//==========ON,OFF MRelay4==========//
void bt5PushCallback(void *ptr) {
  if (statusbt5 == false) {
    pool_size1 = node2.writeSingleRegister(0x04,0x0100); 
    //relayControl_modbusRTU(2, 4, 1);
    //digitalWrite(sw1, LOW);
    statusbt5 = true;
  } else if (statusbt5 == true) {
    pool_size1 = node2.writeSingleRegister(0x04,0x0200);
    //relayControl_modbusRTU(2, 4, 0);
    //digitalWrite(sw1, HIGH);
    statusbt5 = false;
  }
}
//==========ON,OFF MRelay4==========//

//==========ON,OFF MRelay5==========//
void bt6PushCallback(void *ptr) {
  if (statusbt6 == false) {
    pool_size1 = node2.writeSingleRegister(0x05,0x0100);
    //relayControl_modbusRTU(2, 5, 1);
    //digitalWrite(sw1, LOW);
    statusbt6 = true;
  } else if (statusbt6 == true) {
    pool_size1 = node2.writeSingleRegister(0x05,0x0200);
    //relayControl_modbusRTU(2, 5, 0);
    //digitalWrite(sw1, HIGH);
    statusbt6 = false;
  }
}
//==========ON,OFF MRelay5==========//

//==========ON,OFF MRelay6==========//
void bt7PushCallback(void *ptr) {
  if (statusbt7 == false) {
    pool_size1 = node2.writeSingleRegister(0x06,0x0100);
    //relayControl_modbusRTU(2, 6, 1);
    //digitalWrite(sw1, LOW);
    statusbt7 = true;
  } else if (statusbt7 == true) {
    pool_size1 = node2.writeSingleRegister(0x06,0x0200);
    //relayControl_modbusRTU(2, 6, 0);
    //digitalWrite(sw1, HIGH);
    statusbt7 = false;
  }
}
//==========ON,OFF MRelay6==========//

//==========ON,OFF MRelay7==========//
void bt8PushCallback(void *ptr) {
  if (statusbt8 == false) {
    pool_size1 = node2.writeSingleRegister(0x07,0x0100);
    //relayControl_modbusRTU(2, 7, 1);
    //digitalWrite(sw1, LOW);
    statusbt8 = true;
  } else if (statusbt8 == true) {
    pool_size1 = node2.writeSingleRegister(0x07,0x0200);
    //relayControl_modbusRTU(2, 7, 0);
    //digitalWrite(sw1, HIGH);
    statusbt8 = false;
  }
}
//==========ON,OFF MRelay7==========//

//==========ON,OFF MRelay8==========//
void bt9PushCallback(void *ptr) {
  if (statusbt9 == false) {
    pool_size1 = node2.writeSingleRegister(0x08,0x0100);
    //relayControl_modbusRTU(2, 8, 1);
    //digitalWrite(sw1, LOW);
    statusbt9 = true;
  } else if (statusbt9 == true) {
    pool_size1 = node2.writeSingleRegister(0x08,0x0200);
    //relayControl_modbusRTU(2, 8, 0);
    //digitalWrite(sw1, HIGH);
    statusbt9 = false;
  }
}
//==========ON,OFF MRelay8==========//
//==========Modbus Relay============//

//==========Setup function==========//
void setup() {
  Serial.begin(9600);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);//IO16,IO17
  node2.begin(2, Serial2);

  nexInit();                                  //Inital the tft

  // Register the push/pop event callback function
  //Modbus Relay
  bt2.attachPush(bt2PushCallback, &bt2);
  bt3.attachPush(bt3PushCallback, &bt3);
  bt4.attachPush(bt4PushCallback, &bt4);
  bt5.attachPush(bt5PushCallback, &bt5);
  bt6.attachPush(bt6PushCallback, &bt6);
  bt7.attachPush(bt7PushCallback, &bt7);
  bt8.attachPush(bt8PushCallback, &bt8);
  bt9.attachPush(bt9PushCallback, &bt9);

}
//==========Setup function==========//

//==========Loop function===========//
void loop() {
  //When push/pop event occured execute component in touch event list
  nexLoop(nex_listen_list);
}
//==========Loop function===========//
