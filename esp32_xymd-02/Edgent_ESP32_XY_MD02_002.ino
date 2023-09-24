/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
 *************************************************************
  Blynk.Edgent implements:
  - Blynk.Inject - Dynamic WiFi credentials provisioning
  - Blynk.Air    - Over The Air firmware updates
  - Device state indication using a physical LED
  - Credentials reset using a physical Button
 *************************************************************/

/* Fill in information from your Blynk Template here */
/* Read more: https://bit.ly/BlynkInject */
#define BLYNK_TEMPLATE_ID "" //แก้ไข Template ID
#define BLYNK_TEMPLATE_NAME "" //แก้ไข Template Name

#define BLYNK_FIRMWARE_VERSION "0.2.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_ESP32_DEV_MODULE
//#define USE_ESP32C3_DEV_MODULE
//#define USE_ESP32S2_DEV_KIT
#define USE_WROVER_BOARD
//#define USE_TTGO_T7
//#define USE_TTGO_T_OI

#include "BlynkEdgent.h"

//=============Simple Timer=============//
#include <SimpleTimer.h>
SimpleTimer timer;
//=============Simple Timer=============//
//============Modbus Master=============//
#include <ModbusMaster.h>
//#define RX2 16  //RO,RX
//#define TX2 17  //DI,TX
#define RX2 26  //RO,RX
#define TX2 27  //DI,TX
//============Modbus Master=============//
//=============Modbus Object============//
ModbusMaster node1;  //XY-MD02 Slave ID1
ModbusMaster node2;  //XY-MD02 Slave ID2
//=============Modbus Object============//

//=========IO connect to relay==========//
#define relay1 32
#define relay2 33
//=========IO connect to relay==========//

void setup() {
  Serial.begin(115200);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);  //RX2=16 ,TX2=17
  node1.begin(1, Serial2);                    //XY-MD02 Slave ID1
  node2.begin(2, Serial2);                    //XY-MD02 Slave ID2

  //Set Mode
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  //Set default status
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);

  delay(100);

  BlynkEdgent.begin();

  timer.setInterval(1000, xy1);
  timer.setInterval(1000, xy2);
}

//===============XY-MD02-1==============//
void xy1() {
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
  Blynk.virtualWrite(V3, temp1);
  Blynk.virtualWrite(V4, humi1);
}
//===============XY-MD02-1==============//
//===============XY-MD02-2==============//
void xy2() {
  uint8_t result2;
  float temp2 = (node2.getResponseBuffer(0) / 10.0f);
  float humi2 = (node2.getResponseBuffer(1) / 10.0f);
  Serial.println("Get XY-MD02 Data2");
  result2 = node2.readInputRegisters(0x0001, 2);  // Read 2 registers starting at 1)
  if (result2 == node2.ku8MBSuccess) {
    Serial.print("Temp2: ");
    Serial.println(node2.getResponseBuffer(0) / 10.0f);
    Serial.print("Humi2: ");
    Serial.println(node2.getResponseBuffer(1) / 10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V5, temp2);
  Blynk.virtualWrite(V6, humi2);
}
//===============XY-MD02-2==============//


void loop() {
  BlynkEdgent.run();
  timer.run();
}

//==========BTN Blynk ON-OFF Relay1=============//
BLYNK_WRITE(V1) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(relay1, LOW);
  } else {
    digitalWrite(relay1, HIGH);
  }
}
//==========BTN Blynk ON-OFF Relay1============//

//==========BTN Blynk ON-OFF Relay2============//
BLYNK_WRITE(V2) {
  int valuebtn2 = param.asInt();
  if (valuebtn2 == 1) {
    digitalWrite(relay2, LOW);
  } else {
    digitalWrite(relay2, HIGH);
  }
}
//==========BTN Blynk ON-OFF Relay2============//
