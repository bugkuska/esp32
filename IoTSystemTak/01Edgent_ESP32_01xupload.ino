/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 15 Oct, 2022
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
1.Blynk https://github.com/blynkkk/blynk-library 1.0.x
2.ModbusMaster https://github.com/4-20ma/ModbusMaster
3.ModbusRTU (download folder)
4.SimpleTimer https://github.com/jfturcot/SimpleTimer 
5.NTPClient (download folder)
6.LiquidCrystal_i2c (download folder) (Optional)
 *************************************************************************************************/
// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
#define APP_DEBUG
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"

//==============GPIO Pin Map===============//
//Relay1      23
//Relay2      25
//Relay3      33
//Relay4      32
//Ledblynk    18
//RX2         16
//TX2         17
//SCA         21
//SCL         22
//==============GPIO Pin Map===============//
//============Blynk Virtual Pin============//
//V1    SW1
//V2    SW2
//V3    SW3
//V4    SW4
//V5    Humi1
//V6    Temp1
//V7    CurrentDate
//V8    CurrentTime
//============Blynk Virtual Pin============//

//==============ปุ่ม เปิด-ปิด sw1==============//
#define Relay1_sw1 23
#define Widget_Btn_sw1 V1
//==============ปุ่ม เปิด-ปิด sw1==============//

//=============ปุ่ม เปิด-ปิด sw2===============//
#define Relay2_sw2 25
#define Widget_Btn_sw2 V2
//=============ปุ่ม เปิด-ปิด sw2===============//

//============ปุ่ม เปิด-ปิด sw3================//
#define Relay3_sw3 33
#define Widget_Btn_sw3 V3
//============ปุ่ม เปิด-ปิด sw3================//

//============ปุ่ม เปิด-ปิด sw3================//
#define Relay4_sw4 32
#define Widget_Btn_sw4 V4
//============ปุ่ม เปิด-ปิด sw3================//

//==MCU Digital Pin Check blynk connected==//
#define ledblynk 18
//==MCU Digital Pin Check blynk connected==//


//===============SimpleTimer===============//
#include <SimpleTimer.h>
SimpleTimer timer;
//===============SimpleTimer===============//

//==============Modbus-Master==============//
#include <ModbusMaster.h>
#define RXD2 16
#define TXD2 17
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 XY-MD02
//==============Modbus-Master==============//

//=============Button Status===============//
//SW1
boolean stateled1 = 0;
boolean prevStateled1 = 0;
//SW2
boolean stateled2 = 0;
boolean prevStateled2 = 0;
//SW3
boolean stateled3 = 0;
boolean prevStateled3 = 0;
//SW4
boolean stateled4 = 0;
boolean prevStateled4 = 0;

//=============Button Status===============//

//================LCD 2004=================//
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
//================LCD 2004=================//

//=================NTP=====================//
#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
//=================NTP=====================//

//==============Setup Function=============//
void setup() {
  Serial.begin(115200);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node1.begin(1, Serial2);

  //LCD 2004
  // initialize LCD
  lcd.begin();
  // turn on LCD backlight
  lcd.backlight();

  // Setup Pin Mode
  pinMode(Relay1_sw1, OUTPUT);  // ESP32 PIN gpio23
  pinMode(Relay2_sw2, OUTPUT);  // ESP32 PIN GPIO25
  pinMode(Relay3_sw3, OUTPUT);  // ESP32 PIN GPIO33
  pinMode(Relay4_sw4, OUTPUT);  // ESP32 PIN GPIO32
  pinMode(ledblynk, OUTPUT);    // ESP32 PIN GPIO18

  // Set Defult Relay Status
  digitalWrite(Relay1_sw1, LOW);  // ESP32 PIN gpio23
  digitalWrite(Relay2_sw2, LOW);  // ESP32 PIN GPIO25
  digitalWrite(Relay3_sw3, LOW);  // ESP32 PIN GPIO33
  digitalWrite(Relay4_sw4, LOW);  // ESP32 PIN GPIO32
  digitalWrite(ledblynk, HIGH);    // ESP32 PIN GPIO18


  timer.setInterval(5000L, wtr01);               //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L, checkphysic_btn_state);  //เช็คสถานะปุ่มทุกๆ 5 วินาที
  timer.setInterval(30000L, datetime);              //Sync time every 30 sec
  BlynkEdgent.begin();
}
//==============Setup Function=============//

//==============Blynk Connected============//
BLYNK_CONNECTED() {
  if (Blynk.connected()) {
    Serial.println("Blynk Connected");
    digitalWrite(ledblynk, LOW);
    Blynk.syncAll();
  }
}
//==============Blynk Connected============//

//================WTR10-E Sensor===========//
void wtr01() {
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float humi1 = (node1.getResponseBuffer(1) / 10.0f);

  Serial.println("get data1");
  result1 = node1.readHoldingRegisters(0x0000, 2);  // Read 2 registers starting at 1)
  if (result1 == node1.ku8MBSuccess) {
    Serial.print("Temp1: ");
    Serial.println(node1.getResponseBuffer(0) / 10.0f);
    Serial.print("Humi1: ");
    Serial.println(node1.getResponseBuffer(1) / 10.0f);
  }
  delay(1000);

  Blynk.virtualWrite(V5, humi1);
  Blynk.virtualWrite(V6, temp1);
}
//================WTR10-E Sensor===========//

//==========Display NTP Date&Time==========//
void datetime() {
  timeClient.begin();
  //Week Days
  String weekDays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
  //Month names
  String months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec" };
  // Initialize a NTPClient to get time
  //timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200);
  //while (!timeClient.update()) {
  // timeClient.forceUpdate();
  //}
  timeClient.update();
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  unsigned long epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);

  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);

  int currentHour = timeClient.getHours();
  Serial.print("Hour: ");
  Serial.println(currentHour);

  int currentMinute = timeClient.getMinutes();
  Serial.print("Minutes: ");
  Serial.println(currentMinute);

  int currentSecond = timeClient.getSeconds();
  Serial.print("Seconds: ");
  Serial.println(currentSecond);

  String weekDay = weekDays[timeClient.getDay()];
  Serial.print("Week Day: ");
  Serial.println(weekDay);

  String currentTime = (String)currentHour + ":" + (String)currentMinute + ":" + (String)currentSecond;

  Blynk.virtualWrite(V8, currentTime);

  //Get a time structure
  struct tm *ptm = gmtime((time_t *)&epochTime);

  int monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);

  int currentMonth = ptm->tm_mon + 1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  String currentMonthName = months[currentMonth - 1];
  Serial.print("Month name: ");
  Serial.println(currentMonthName);

  int currentYear = ptm->tm_year + 1900;
  Serial.print("Year: ");
  Serial.println(currentYear);

  //Print complete date:
  String currentDate = (String)monthDay + ":" + (String)currentMonthName + ":" + (String)currentYear;
  Serial.print("Current date: ");
  Serial.println(currentDate);
  Serial.println();
  Blynk.virtualWrite(V7, currentDate);
}
//==========Display NTP Date&Time==========//

//===============Manual Switch1============//
BLYNK_WRITE(Widget_Btn_sw1){
      int valuebtn1 = param.asInt();
      if(valuebtn1 == 1){
        digitalWrite(Relay1_sw1,HIGH);        
      }
       else{              
        digitalWrite(Relay1_sw1,LOW);        
     }
}
//===============Manual Switch1============//

//===============Manual Switch2============//
BLYNK_WRITE(Widget_Btn_sw2){
      int valuebtn2 = param.asInt();
      if(valuebtn2 == 1){
        digitalWrite(Relay2_sw2,HIGH);        
      }
       else{              
        digitalWrite(Relay2_sw2,LOW);        
     }
}
//===============Manual Switch2============//

//===============Manual Switch3============//
BLYNK_WRITE(Widget_Btn_sw3){
      int valuebtn3 = param.asInt();
      if(valuebtn3 == 1){
        digitalWrite(Relay3_sw3,HIGH);        
      }
       else{              
        digitalWrite(Relay3_sw3,LOW);        
     }
}
//===============Manual Switch3============//

//===============Manual Switch4============//
BLYNK_WRITE(Widget_Btn_sw4){
      int valuebtn4 = param.asInt();
      if(valuebtn4 == 1){
        digitalWrite(Relay4_sw4,HIGH);        
      }
       else{              
        digitalWrite(Relay4_sw4,LOW);        
     }
}
//===============Manual Switch4============//


//========Check Status LED Widget===========//
void checkphysic_btn_state() {
  stateled1 = digitalRead(Relay1_sw1);  //Check ON/OFF สวิตส์1
  if (stateled1 != prevStateled1) {
    if (stateled1 == 1) Blynk.virtualWrite(V1, 1);
    if (stateled1 == 0) Blynk.virtualWrite(V1, 0);
  }
  prevStateled1 = stateled1;

  stateled2 = digitalRead(Relay2_sw2);  //Check ON/OFF สวิตส์2
  if (stateled2 != prevStateled2) {
    if (stateled2 == 1) Blynk.virtualWrite(V2, 1);
    if (stateled2 == 0) Blynk.virtualWrite(V2, 0);
  }
  prevStateled2 = stateled2;

  stateled3 = digitalRead(Relay3_sw3);  //Check ON/OFF สวิตส์3
  if (stateled3 != prevStateled3) {
    if (stateled3 == 1) Blynk.virtualWrite(V3, 1);
    if (stateled3 == 0) Blynk.virtualWrite(V3, 0);
  }
  prevStateled3 = stateled3;

  stateled4 = digitalRead(Relay4_sw4);  //Check ON/OFF สวิตส์4
  if (stateled4 != prevStateled4) {
    if (stateled4 == 1) Blynk.virtualWrite(V4, 1);
    if (stateled4 == 0) Blynk.virtualWrite(V4, 0);
  }
  prevStateled4 = stateled4;
}
//========Check Status LED Widget===========//

//==============Loop Function==============//
void loop() {
  //Display date
  timeClient.begin();
  String weekDays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
  String months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec" };
  timeClient.setTimeOffset(25200);
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  String weekDay = weekDays[timeClient.getDay()];
  String currentTime = (String)currentHour + ":" + (String)currentMinute + ":" + (String)currentSecond;
  //Get a time structure
  struct tm *ptm = gmtime((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  String currentMonthName = months[currentMonth - 1];
  int currentYear = ptm->tm_year + 1900;
  //Print complete date:
  String currentDate = (String)monthDay + ":" + (String)currentMonthName + ":" + (String)currentYear;

  //LCD 2004
  //lcd.begin();
  //lcd.backlight();

  lcd.setCursor(2, 0);
  lcd.print("*FB/SMFTHAILAND*");

  lcd.setCursor(2, 1);
  lcd.print("DATE:");
  lcd.print(currentDate);

  //LCD2004 Display
  lcd.setCursor(0, 2);
  lcd.print("C1:");
  if (stateled1 == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF ");
  }
  prevStateled1 = stateled1;

  lcd.setCursor(7, 2);
  lcd.print("C2:");
  if (stateled2 == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }
  prevStateled2 = stateled2;

  lcd.setCursor(14, 2);
  lcd.print("C3:");
  if (stateled3 == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }
  prevStateled3 = stateled3;

  lcd.setCursor(0, 3);
  lcd.print("C4:");
  if (stateled4 == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF ");
  }
  prevStateled4 = stateled4;

  BlynkEdgent.run();
  timer.run();
}
//==============Loop Function==============//
