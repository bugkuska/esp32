/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 6 Mar, 2023
    Update On: 1 July, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    1. Master Node : ESP32+TTL485+Transciever
    2. Slave Node + Transciever
      Slave ID1 : XY-MD02 Temperature & Humidity
      Slave ID2 : Soil Moisture Sensor
    3. Blynk.Edgent + Multi-tasking Programing
    4. Display data via I2C LCD 
    5. Display data & control via Blynk webdashboard & blynk mobile application (Auto/Manual)
    6. Reset button & Reconfigure Wi-Fi from blynk mobile application 
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1. Blynk Library https://github.com/blynkkk/blynk-library/archive/refs/heads/master.zip
    2. Modbus Master https://github.com/4-20ma/ModbusMaster/archive/refs/heads/master.zip
    3. LiquidCrystal_I2C  https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library/archive/refs/heads/master.zip
  *  *********************************************************************************************
*/
// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
#define APP_DEBUG
// Uncomment your board, or configure a custom board in Settings.h
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"

//===============SimpleTimer=================//
#include <SimpleTimer.h>
SimpleTimer timer;
//===============SimpleTimer=================//

//================Relay State================//
boolean stateled1 = 0;
boolean stateled2 = 0;
boolean stateled3 = 0;
boolean stateled4 = 0;
boolean prevStateled1 = 0;
boolean prevStateled2 = 0;
boolean prevStateled3 = 0;
boolean prevStateled4 = 0;
//================Relay State================//

//=========Define IO connect to relay========//
#define relay1 25        //Relay channel1
bool switchStatus1 = 0;  // 0 = manual,1=auto
int TempLevelLimit1 = 0;
bool manualSwitch1 = 0;

#define relay2 26  //Relay channel2
#define relay3 33  //Relay channel3
#define relay4 32  //Relay channel4
//=========Define IO connect to relay========//
//=================LCD 2004==================//
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
//=================LCD 2004==================//

//==============Modbus-Master===============//
#include <ModbusMaster.h>
#define RXD2 16
#define TXD2 17
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 XY-MD02
float humi1 = 0.0f;
float temp1 = 0.0f;
ModbusMaster node2;  //Serial2 SlaveID2 SoilMoisture
//==============Modbus-Master===============//

// สร้างตัวแปรเก็บค่า Tick ที่แปลงจากเวลาที่เป็น millisecond
const TickType_t xDelay1000ms = pdMS_TO_TICKS(1000);
const TickType_t xDelay2000ms = pdMS_TO_TICKS(2000);
const TickType_t xDelay5000ms = pdMS_TO_TICKS(5000);
// สร้างตัวแปร TaskHandle สำหรับแต่ละ Task
TaskHandle_t Task1 = NULL;
TaskHandle_t Task2 = NULL;
TaskHandle_t Task3 = NULL;
// สร้างตัวแปรเก็บค่าเป็น integer เพื่อเอาไปใช้ใน Task
int passValue = 0;

//===============Setup function==============//
void setup() {
  Serial.begin(9600);                           // เริ่มต้น Serial port เพื่อดูค่าที่แต่ละ Task ปริ้นออกมา
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  //RX2=16,RO ,TX2=17, DI
  node1.begin(1, Serial2);
  node2.begin(2, Serial2);

  // Set IO pinMode for relay
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  //Set default relay status at boot
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);

  //LCD 2004
  lcd.begin();      // initialize LCD
  lcd.backlight();  // turn on LCD backlight

  delay(1000);
  // สร้าง Task1 โดยใช้ฟังก์ชัน func1_Task() พร้อมทั้งส่งผ่านค่าตัวแปร PassVale เข้าไปคำนวนใน Task
  xTaskCreatePinnedToCore(func1_Task, "Task1", 10000, (void *)passValue, 1, &Task1, 0);
  // สร้าง Task2 โดยใช้ฟังก์ชัน func2_Task()
  xTaskCreatePinnedToCore(func2_Task, "Task2", 10000, NULL, 1, &Task2, 0);
  // สร้าง Task3 โดยใช้ฟังก์ชัน func3_Task()
  xTaskCreatePinnedToCore(func3_Task, "Task3", 10000, NULL, 1, &Task3, 0);

  delay(100);
  BlynkEdgent.begin();
  timer.setInterval(5000L, checkphysic_btn_state);
}
//===============Setup function==============//

//===============Blynk conneted==============//
BLYNK_CONNECTED() {
  if (Blynk.connected()) {
    Serial.println("Blynk connected");
    Blynk.syncAll();
  }
}
//===============Blynk conneted==============//

//========Check Status LED Widget============//
void checkphysic_btn_state() {
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

  stateled4 = digitalRead(relay4);  //Check ON/OFF สวิตส์4
  if (stateled4 != prevStateled4) {
    if (stateled4 == 1) Blynk.virtualWrite(V4, 1);
    if (stateled4 == 0) Blynk.virtualWrite(V4, 0);
  }
  prevStateled4 = stateled4;
}
//========Check Status LED Widget============//
void loop() {
  //LCD2004 Display
  float temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float humi1 = (node1.getResponseBuffer(1) / 10.0f);
  float soil1 = (node2.getResponseBuffer(2) / 10.0f);

  lcd.setCursor(2, 0);
  lcd.print("Soil1: ");
  lcd.print(soil1);

  lcd.setCursor(15, 0);
  lcd.print(" %");

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
  prevStateled1 = stateled1;

  lcd.setCursor(11, 2);
  lcd.print("CH2:");
  if (stateled2 == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }
  prevStateled2 = stateled2;

  lcd.setCursor(2, 3);
  lcd.print("CH3:");
  if (stateled3 == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }
  prevStateled3 = stateled3;

  lcd.setCursor(11, 3);
  lcd.print("CH4:");
  if (stateled4 == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF ");
  }
  prevStateled4 = stateled4;

  timer.run();
}
//===============Loop function===============//

//====Auto/Manual Temperature ON/OFF SW2=====//
BLYNK_WRITE(V8) {
  switchStatus1 = param.asInt();  // Get value as integer
}
//====Auto/Manual Temperature ON/OFF SW2=====//

//=====Slider Update Temperature setting=====//
BLYNK_WRITE(V9) {
  TempLevelLimit1 = param.asInt();  // Get value as integer
}
//=====Slider Update Temperature setting=====//

//=============== ManualSwitch1==============//
BLYNK_WRITE(V1) {
  manualSwitch1 = param.asInt();
}
//=============== ManualSwitch1==============//

//===========funtion ที่ทำงานใน Task1=========//
void func1_Task(void *pvvalue) {
  int f1param = (int)pvvalue;  //แปลงค่าตัวแปรที่ส่งผ่านมาเป็น Integer
  while (1) {
    Serial.println(String("Task1 : count >> ") + f1param);
    Serial.println(String("Slave ID1"));
    f1param++;

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

    if (switchStatus1) {
      // auto
      if (temp1 > TempLevelLimit1) {
        digitalWrite(relay1, HIGH);
        Blynk.virtualWrite(V1, 1);
      } else {
        digitalWrite(relay1, LOW);
        Blynk.virtualWrite(V1, 0);
      }
    } else {
      if (manualSwitch1) {
        digitalWrite(relay1, HIGH);
      } else {
        digitalWrite(relay1, LOW);
      }
      // manaul
    }
  }
  vTaskDelay(xDelay5000ms);  // Delay Task นี้ 5 วินาที
}
//===========funtion ที่ทำงานใน Task1=========//

//===========funtion ที่ทำงานใน Task2=========//
void func2_Task(void *pvParam) {
  while (1) {
    Serial.println(String("Task2 Slave ID2"));
    uint8_t result2;
    float soil1 = (node2.getResponseBuffer(2) / 10.0f);

    Serial.println("Get Soil Moisture Data:");
    result2 = node2.readHoldingRegisters(0x0000, 3);  // Read 2 registers starting at 1)
    if (result2 == node2.ku8MBSuccess) {
      Serial.print("Soil Moisture: ");
      Serial.println(node2.getResponseBuffer(2) / 10.0f);
    }
    delay(1000);
    Blynk.virtualWrite(V7, soil1);
    vTaskDelay(xDelay5000ms);  // Delay Task นี้ 5 วินาที
  }
}
//===========funtion ที่ทำงานใน Task2=========//

//===========funtion ที่ทำงานใน Task3=========//
void func3_Task(void *pvParam) {
  while (1) {
    BlynkEdgent.run();
    vTaskDelay(xDelay1000ms);  // Delay Task นี้ 1 วินาที
  }
}
//===========funtion ที่ทำงานใน Task3=========//

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
