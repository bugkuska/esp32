/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 16 July, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    1. One Modbus RTU 
      Slave ID1 : XY-MD02 Temperature & Humidity
    2. Multi-tasking Programing
    3. Display data & relay status on LCD2004
    4. Display data & control via Blynk webdashboard & blynk mobile application
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1. Blynk Library https://github.com/blynkkk/blynk-library/archive/refs/heads/master.zip
    2. Modbus Master https://github.com/4-20ma/ModbusMaster/archive/refs/heads/master.zip
  *  *********************************************************************************************
    How to
    1. Create file from example
    File > Examples > Blynk > Blynk.Edgent > Edgent_ESP32
    2. Copy source-code from github and replace .ino fils
 *************************************************************************************************/
// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID ""              //Template id from blynk.console
#define BLYNK_DEVICE_NAME "esp32r4"       //Template name from blynk.console
#define BLYNK_FIRMWARE_VERSION "0.1.0"    //Firmware version
#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
#define APP_DEBUG
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"
bool fetch_blynk_state = true;  //true or false
//==============SimpleTimer==================//
#include <SimpleTimer.h>
SimpleTimer timer;
//==============SimpleTimer==================//
//=========Define IO connect to relay========//
#define relay1 25     //Relay channel1
#define relay2 33     //Relay channel2
#define relay3 32     //Relay channel3
#define relay4 23     //Relay channel4
#define ledblynk 18   //Map to Blynk virtual pin V1
//=========Define IO connect to relay========//
//================Relay State================//
boolean stateled1 = 0;
boolean prevStateled1 = 0;
boolean stateled2 = 0;
boolean prevStateled2 = 0;
boolean stateled3 = 0;
boolean prevStateled3 = 0;
boolean stateled4 = 0;
boolean prevStateled4 = 0;
//================Relay State================//
// สร้างตัวแปรเก็บค่า Tick ที่แปลงจากเวลาที่เป็น millisecond
const TickType_t xDelay2000ms = pdMS_TO_TICKS(2000);
const TickType_t xDelay10000ms = pdMS_TO_TICKS(10000);
// สร้างตัวแปร TaskHandle สำหรับแต่ละ Task
TaskHandle_t Task1 = NULL;
TaskHandle_t Task2 = NULL;
//=================LCD 2004==================//
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
//=================LCD 2004==================//
//==============Modbus-Master================//
#include <ModbusMaster.h>
#define RXD2 16
#define TXD2 17
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 XY-MD02
float humi1 = 0.0f;
float temp1 = 0.0f;
//==============Modbus-Master================//
//==============Blynk Connected==============//
BLYNK_CONNECTED() {
  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    Serial.println("Blynk Not Connected");
    digitalWrite(ledblynk, HIGH);
  }
  if (isconnected == true) {
    if (fetch_blynk_state) {
      digitalWrite(ledblynk, LOW);
      Blynk.syncAll();
    }
  }
}
//==============Blynk Connected==============//
//===============Setup function==============//
void setup() {
  Serial.begin(9600);                           // เริ่มต้น Serial port เพื่อดูค่าที่แต่ละ Task ปริ้นออกมา
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  //RX2=16,RO ,TX2=17, DI
  node1.begin(1, Serial2);
  // Set IO pinMode for relay
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(ledblynk, OUTPUT);
  //Set default relay status at boot
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  digitalWrite(ledblynk, HIGH);
  //LCD 2004
  lcd.begin();      // initialize LCD
  lcd.backlight();  // turn on LCD backlight
  BlynkEdgent.begin();
  delay(1000);
  if (!fetch_blynk_state) {
  }
  /*Task function
  String with name of task
  Stack size in words
  Parameter passed as input of the task
  Priority of the task
  Task handle
  Core  
  */
  // สร้าง Task1 โดยใช้ฟังก์ชัน func1_Task()
  xTaskCreatePinnedToCore(func1_Task, "Task1", 10000, NULL, 1, &Task1, 1);
  // สร้าง Task2 โดยใช้ฟังก์ชัน func2_Task()
  xTaskCreatePinnedToCore(func2_Task, "Task2", 10000, NULL, 2, &Task1, 1);
}
//===============Setup function==============//
//=========BTN Blynk ON-OFF Relay1===========//
BLYNK_WRITE(V1) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(relay1, HIGH);
  } else {
    digitalWrite(relay1, LOW);
  }
}
//=========BTN Blynk ON-OFF Relay1===========//
//=========BTN Blynk ON-OFF Relay2===========//
BLYNK_WRITE(V2) {
  int valuebtn2 = param.asInt();
  if (valuebtn2 == 1) {
    digitalWrite(relay2, HIGH);
  } else {
    digitalWrite(relay2, LOW);
  }
}
//=========BTN Blynk ON-OFF Relay2===========//
//=========BTN Blynk ON-OFF Relay3===========//
BLYNK_WRITE(V3) {
  int valuebtn3 = param.asInt();
  if (valuebtn3 == 1) {
    digitalWrite(relay3, HIGH);
  } else {
    digitalWrite(relay3, LOW);
  }
}
//=========BTN Blynk ON-OFF Relay3===========//
//=========BTN Blynk ON-OFF Relay4===========//
BLYNK_WRITE(V4) {
  int valuebtn4 = param.asInt();
  if (valuebtn4 == 1) {
    digitalWrite(relay4, HIGH);
  } else {
    digitalWrite(relay4, LOW);
  }
}
//=========BTN Blynk ON-OFF Relay4===========//
//===============Loop function===============//
void loop() {
  BlynkEdgent.run();
}
//===============Loop function===============//

//===========funtion ที่ทำงานใน Task1==========//
void func1_Task(void *pvParam) {
  while (1) {
    Serial.println(String("Task1: Display Data on LCD2004"));
    stateled1 = digitalRead(relay1);  //Check ON/OFF สวิตส์1
    if (stateled1 != prevStateled1) {
      if (stateled1 == 1) Blynk.virtualWrite(V1, 1);
      if (stateled1 == 0) Blynk.virtualWrite(V1, 0);
    }
    prevStateled1 = stateled1;

    stateled2 = digitalRead(relay2);  //Check ON/OFF สวิตส์2
    if (stateled2 != prevStateled2) {
      if (stateled2 == 1) Blynk.virtualWrite(V2, 1);
      if (stateled2 == 0) Blynk.virtualWrite(V2, 0);
    }
    prevStateled2 = stateled2;

    stateled3 = digitalRead(relay3);  //Check ON/OFF สวิตส์3
    if (stateled3 != prevStateled3) {
      if (stateled3 == 1) Blynk.virtualWrite(V3, 1);
      if (stateled3 == 0) Blynk.virtualWrite(V3, 0);
    }
    prevStateled3 = stateled3;

    stateled4 = digitalRead(relay4);
    ;  //Check ON/OFF สวิตส์4
    if (stateled4 != prevStateled4) {
      if (stateled4 == 1) Blynk.virtualWrite(V4, 1);
      if (stateled4 == 0) Blynk.virtualWrite(V4, 0);
    }
    prevStateled4 = stateled4;

    //LCD2004 Display
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
    lcd.setCursor(1, 0);
    lcd.print("***SMFThailand***");

    lcd.setCursor(0, 1);
    lcd.print("TEM:");
    lcd.print(temp1);

    lcd.setCursor(11, 1);
    lcd.print("HUM:");
    lcd.print(humi1);

    lcd.setCursor(2, 2);
    lcd.print("CH1:");
    if (stateled1 == 1) {
      lcd.print("ON ");
    } else {
      lcd.print("OFF ");
    }

    lcd.setCursor(11, 2);
    lcd.print("CH2:");
    if (stateled2 == 1) {
      lcd.print("ON ");
    } else {
      lcd.print("OFF");
    }

    lcd.setCursor(2, 3);
    lcd.print("CH3:");
    if (stateled3 == 1) {
      lcd.print("ON ");
    } else {
      lcd.print("OFF");
    }

    lcd.setCursor(11, 3);
    lcd.print("CH4:");
    if (stateled4 == 1) {
      lcd.print("ON ");
    } else {
      lcd.print("OFF ");
    }
    vTaskDelay(xDelay2000ms);  // Delay Task นี้ 2 วินาที
  }
}
//===========funtion ที่ทำงานใน Task1==========//
//===========funtion ที่ทำงานใน Task2==========//
void func2_Task(void *pvParam) {
  while (1) {
    Serial.println(String("Task2: XY-MD02 to Blynk 2.0"));
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
    delay(1000);
    Blynk.virtualWrite(V5, temp1);
    Blynk.virtualWrite(V6, humi1);
    vTaskDelay(xDelay10000ms);  // Delay Task นี้ 2 วินาที
  }
}
//===========funtion ที่ทำงานใน Task2==========//
