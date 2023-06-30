/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 6 Mar, 2023
    Update On: 30 June, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    1. Master Node : ESP32+TTL485+Transciever
    2. Slave Node + Transciever
      Slave ID1 : XY-MD02 Temperature & Humidity
      Slave ID2 : Soil Moisture Sensor
    3. Blynk.Edgent + Multi-tasking Programing
    4. Display data via I2C LCD 
    5. Display data & control via Blynk webdashboard & blynk mobile application
    6. Reset button & Reconfigure Wi-Fi from blynk mobile application 
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1. Blynk Library https://github.com/blynkkk/blynk-library/archive/refs/heads/master.zip
    2. Modbus Master https://github.com/4-20ma/ModbusMaster/archive/refs/heads/master.zip
    3. ezButton https://github.com/ArduinoGetStarted/button/archive/refs/heads/master.zip
    4. LiquidCrystal_I2C  https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library/archive/refs/heads/master.zip
  *  *********************************************************************************************
*/
// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPL6fX-QY3S9"
#define BLYNK_DEVICE_NAME "esp32r4"
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
#define APP_DEBUG
// Uncomment your board, or configure a custom board in Settings.h
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"

//==================EEPROM===================//
#include <EEPROM.h>
#define EEPROM_SIZE 4
//==================EEPROM===================//
//==================zButton==================//
#include <ezButton.h>
ezButton button1(34);  //Manual button1
ezButton button2(35);  //Manual button2
ezButton button3(36);  //Manual button3
ezButton button4(39);  //Manual button4
//==================zButton==================//
//================Relay State================//
bool state_relay1 = LOW;
bool state_relay2 = LOW;
bool state_relay3 = LOW;
bool state_relay4 = LOW;
boolean prevStateled1 = 0;
boolean prevStateled2 = 0;
boolean prevStateled3 = 0;
boolean prevStateled4 = 0;
//================Relay State================//

//=========Define IO connect to relay========//
#define relay1 25  //Relay channel1
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
const TickType_t xDelay2000ms = pdMS_TO_TICKS(2000);
// สร้างตัวแปร TaskHandle สำหรับแต่ละ Task
TaskHandle_t Task1 = NULL;
TaskHandle_t Task2 = NULL;
TaskHandle_t Task3 = NULL;
// สร้างตัวแปรเก็บค่าเป็น integer เพื่อเอาไปใช้ใน Task
int passValue = 0;

//===============Setup function==============//
void setup() {
  Serial.begin(9600);       // เริ่มต้น Serial port เพื่อดูค่าที่แต่ละ Task ปริ้นออกมา
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  //RX2=16,RO ,TX2=17, DI
  node1.begin(1, Serial2);
  node2.begin(2, Serial2);

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

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

  digitalWrite(relay1, !state_relay1);
  digitalWrite(relay2, !state_relay2);
  digitalWrite(relay3, !state_relay3);
  digitalWrite(relay4, !state_relay4);

  state_relay1 = EEPROM.read(0);
  state_relay2 = EEPROM.read(1);
  state_relay3 = EEPROM.read(2);
  state_relay4 = EEPROM.read(3);

  digitalWrite(relay1, state_relay1);
  digitalWrite(relay2, state_relay2);
  digitalWrite(relay3, state_relay3);
  digitalWrite(relay4, state_relay4);

  Serial.printf("relay1 is %s \n", state_relay1 ? "ON" : "OFF");
  Serial.printf("relay2 is %s \n", state_relay2 ? "ON" : "OFF");
  Serial.printf("relay3 is %s \n", state_relay3 ? "ON" : "OFF");
  Serial.printf("relay4 is %s \n", state_relay4 ? "ON" : "OFF");

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
}
//===============Setup function==============//

//===============Loop function===============//
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
  if (state_relay1 == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF ");
  }
  prevStateled1 = state_relay1;

  lcd.setCursor(11, 2);
  lcd.print("CH2:");
  if (state_relay2 == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }
  prevStateled2 = state_relay2;

  lcd.setCursor(2, 3);
  lcd.print("CH3:");
  if (state_relay3 == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }
  prevStateled3 = state_relay3;

  lcd.setCursor(11, 3);
  lcd.print("CH4:");
  if (state_relay4 == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF ");
  }
  prevStateled4 = state_relay4;

  button_control();
  //BlynkEdgent.run();
}
//===============Loop function===============//

//===========funtion ที่ทำงานใน Task1=========//
void func1_Task(void *pvvalue) {
  int f1param = (int)pvvalue;  //แปลงค่าตัวแปรที่ส่งผ่านมาเป็น Integer
  while (1) {
    Serial.println(String("hello from Task1 : count >> ") + f1param);
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
    vTaskDelay(xDelay2000ms);  // Delay Task นี้ 2 วินาที
  }
}
//===========funtion ที่ทำงานใน Task1=========//

//===========funtion ที่ทำงานใน Task2=========//
void func2_Task(void *pvParam) {
  while (1) {
    Serial.println(String("hello from Task2"));
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
    vTaskDelay(xDelay2000ms);  // Delay Task นี้ 2 วินาที
  }
}
//===========funtion ที่ทำงานใน Task2=========//

//===========funtion ที่ทำงานใน Task3=========//
void func3_Task(void *pvParam) {
  while (1) {
    Serial.println(String("hello from Task3"));
    //button_control();
    BlynkEdgent.run();
    vTaskDelay(xDelay2000ms);  // Delay Task นี้ 2 วินาที
  }
}
//===========funtion ที่ทำงานใน Task3=========//

//===========Manual Button control===========//
void button_control() {
  button1.loop();
  if (button1.isPressed()) {
    control_relay(1, relay1, state_relay1);
    //add
    //Blynk.virtualWrite(V1, state_relay1);
  }
  button2.loop();
  if (button2.isPressed()) {
    control_relay(2, relay2, state_relay2);
    //Blynk.virtualWrite(V2, state_relay2);
  }
  button3.loop();
  if (button3.isPressed()) {
    control_relay(3, relay3, state_relay3);
    //Blynk.virtualWrite(V3, state_relay3);
  }
  button4.loop();
  if (button4.isPressed()) {
    control_relay(4, relay4, state_relay4);
    //Blynk.virtualWrite(V4, state_relay4);
  }
}
//===========Manual Button control===========//

//======Manual Button control function=======//
void control_relay(int relay_no, int relay_pin, boolean &status) {
  status = !status;
  digitalWrite(relay_pin, status);
  EEPROM.write(relay_no - 1, status);
  EEPROM.commit();
  String text = (status) ? "ON" : "OFF";
  Serial.println("Relay" + String(relay_no) + " is " + text);
}
//======Manual Button control function=======//

//=========BTN Blynk ON-OFF Relay1===========//
BLYNK_WRITE(V1) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(relay1, HIGH);
    control_relay(1, relay1, state_relay1);
  } else {
    digitalWrite(relay1, LOW);
    control_relay(1, relay1, state_relay1);
  }
}
//=========BTN Blynk ON-OFF Relay1===========//

//=========BTN Blynk ON-OFF Relay2===========//
BLYNK_WRITE(V2) {
  int valuebtn2 = param.asInt();
  if (valuebtn2 == 1) {
    digitalWrite(relay2, HIGH);
    control_relay(2, relay2, state_relay2);
  } else {
    digitalWrite(relay2, LOW);
    control_relay(2, relay2, state_relay2);
  }
}
//=========BTN Blynk ON-OFF Relay2===========//

//=========BTN Blynk ON-OFF Relay3===========//
BLYNK_WRITE(V3) {
  int valuebtn3 = param.asInt();
  if (valuebtn3 == 1) {
    digitalWrite(relay3, HIGH);
    control_relay(3, relay3, state_relay3);
  } else {
    digitalWrite(relay3, LOW);
    control_relay(3, relay3, state_relay3);
  }
}
//=========BTN Blynk ON-OFF Relay3===========//

//=========BTN Blynk ON-OFF Relay4===========//
BLYNK_WRITE(V4) {
  int valuebtn4 = param.asInt();
  if (valuebtn4 == 1) {
    digitalWrite(relay4, HIGH);
    control_relay(4, relay4, state_relay4);
  } else {
    digitalWrite(relay4, LOW);
    control_relay(4, relay4, state_relay4);
  }
}
//=========BTN Blynk ON-OFF Relay4===========//
