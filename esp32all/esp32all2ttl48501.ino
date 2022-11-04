/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 3 Nov, 2022
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    1. Install esp32 
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    2. Create Blynk.Edgent initails files
    File--> Examples--> Blynk--> Blynk.Edgent--> Edgent_ESP32
 *  *********************************************************************************************
    3.Install the following libraries :
    1.Blynk https://github.com/blynkkk/blynk-library 1.0.x
    2.ModbusMaster https://github.com/4-20ma/ModbusMaster
    3.ModbusRTU.h Libraries
    4.SimpleTimer https://github.com/jfturcot/SimpleTimer 
    5.SSD1306Wire.h, DS3231.h
    6.NTPClient
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

//============Onboard Relay================//
#define ledblynk 19  //Relay1
//Relay2  18
//Relay3  5
//Relay4  25
//============Onboard Relay================//

//===============SimpleTimer===============//
#include <SimpleTimer.h>
SimpleTimer timer;
//===============SimpleTimer===============//

//================ModbusRTU================//
#define RXD1 16
#define TXD1 17
HardwareSerial rs485(1);
#include "modbusRTU.h"
//================ModbusRTU================//

//========VirtualPin Modbus Relay==========//
#define Widget_btn_mbr1 V1
#define Widget_btn_mbr2 V2
#define Widget_btn_mbr3 V3
#define Widget_btn_mbr4 V4
#define Widget_btn_mbr5 V5
#define Widget_btn_mbr6 V6
#define Widget_btn_mbr7 V7
#define Widget_btn_mbr8 V8
#define Widget_btn_mbr9 V9
#define Widget_btn_mbr10 V10
#define Widget_btn_mbr11 V11
#define Widget_btn_mbr12 V12
//========VirtualPin Modbus Relay==========//
//V13 XY-MD02 Temperature
//V14 XY-MD02 Humidity
//V15 WTR10-E Temperature
//V16 WTR10-E Humidity
//V17 MCU Temperature
//V18 CurrentTime
//V19 CurrentDate
//========VirtualPin Modbus Relay==========//

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
//SW5
boolean stateled5 = 0;
boolean prevStateled5 = 0;

//SW6
boolean stateled6 = 0;
boolean prevStateled6 = 0;
//SW7
boolean stateled7 = 0;
boolean prevStateled7 = 0;
//SW8
boolean stateled8 = 0;
boolean prevStateled8 = 0;
//SW9
boolean stateled9 = 0;
boolean prevStateled9 = 0;
//SW10
boolean stateled10 = 0;
boolean prevStateled10 = 0;
//SW11
boolean stateled11 = 0;
boolean prevStateled11 = 0;
//SW12
boolean stateled12 = 0;
boolean prevStateled12 = 0;
//=============Button Status===============//

//==============Modbus-Master==============//
#include <ModbusMaster.h>
#define RXD2 26
#define TXD2 27
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 XY-MD02
//==============Modbus-Master==============//

//==============OLED Display===============//
#include <Wire.h>         // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"  // legacy include: `#include "SSD1306.h"`
SSD1306Wire display(0x3c, 21, 22);
//==============OLED Display===============//

//==============LCD I2C 2004===============//
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
//==============LCD I2C 2004===============//

//=================NTP=====================//
#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
//=================NTP=====================//

//=========DS3231 Real Time Clock==========//
#include <DS3231.h>
DS3231 Clock;
//=========DS3231 Real Time Clock==========//

//==============Setup Function=============//
void setup() {
  Serial.begin(115200);
  rs485.begin(9600, SERIAL_8N1, RXD1, TXD1);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node1.begin(1, Serial2);

  //OLED
  display.init();
  display.display();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.display();
  
  //LCD 2004
  // initialize LCD
  lcd.begin();
  // turn on LCD backlight
  lcd.backlight();

  pinMode(ledblynk, OUTPUT);
  digitalWrite(ledblynk, HIGH);

  timer.setInterval(5000L, xymd02);              //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L, wtr10e);              //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L, inside_temperature);  //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L, checkphysic_btn_state); //เช็คสถานะสวิตส์ทุก 5 วินาที
  timer.setInterval(30000L, datetime);  //Sync time every 30 sec
    
  BlynkEdgent.begin();
}
//==============Setup Function=============//

//=========Display the temperature=========//
void inside_temperature() {
  float BoardTemp;
  BoardTemp = Clock.getTemperature();
  Blynk.virtualWrite(V17, BoardTemp);
}
//=========Display the temperature=========//


//==============Blynk Connected============//
BLYNK_CONNECTED() {
  if (Blynk.connected()) {
    Serial.println("Blynk Connected");
    digitalWrite(ledblynk, LOW);
    Blynk.syncAll();
  }
}
//==============Blynk Connected============//

//=========Modbus-Master XY-MD02===========//
void xymd02() {
  uint8_t result1;
  float xy02_temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float xy02_humi1 = (node1.getResponseBuffer(1) / 10.0f);

  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2);  //Function 04, Read 2 registers starting at 2)
  if (result1 == node1.ku8MBSuccess) {
    Serial.print("Temp: ");
    Serial.println(node1.getResponseBuffer(0) / 10.0f);
    Serial.print("Humi: ");
    Serial.println(node1.getResponseBuffer(1) / 10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V13, xy02_temp1);
  Blynk.virtualWrite(V14, xy02_humi1);
}
//=========Modbus-Master XY-MD02===========//

//===========Modbus-RTU wtr10e=============//
void wtr10e() {
  int id1 = 1;
  float wtr10_temp1 = sht20ReadTemp_modbusRTU(id1);
  float wtr10_humi1 = sht20ReadHumi_modbusRTU(id1);

  Serial.printf("Info: sht20[0x01] temperature1 = %.1f\r\n", wtr10_temp1);
  vTaskDelay(1000);
  Serial.printf("Info: sht20[0x01] humidity1 = %.1f\r\n", wtr10_humi1);
  vTaskDelay(1000);

  Blynk.virtualWrite(V15, wtr10_temp1);
  Blynk.virtualWrite(V16, wtr10_humi1);
}
//===========Modbus-RTU wtr10e=============//

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

  Blynk.virtualWrite(V18, currentTime);

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
  Blynk.virtualWrite(V19, currentDate);
  display.drawString(0, 40, "Date :" + String(currentDate));
}
//==========Display NTP Date&Time==========//

//============Modbus-RTU Relay=============//
//============BUTTON ON/OFF mbr1===========//
BLYNK_WRITE(Widget_btn_mbr1) {
  int valuembr1 = param.asInt();
  if (valuembr1 == 1) {
    relayControl_modbusRTU(2, 1, 1);
  } else {
    relayControl_modbusRTU(2, 1, 0);
  }
}
//============BUTTON ON/OFF mbr1===========//

//============BUTTON ON/OFF mbr2===========//
BLYNK_WRITE(Widget_btn_mbr2) {
  int valuembr2 = param.asInt();
  if (valuembr2 == 1) {
    relayControl_modbusRTU(2, 2, 1);
  } else {
    relayControl_modbusRTU(2, 2, 0);
  }
}
//============BUTTON ON/OFF mbr2===========//

//============BUTTON ON/OFF mbr3===========//
BLYNK_WRITE(Widget_btn_mbr3) {
  int valuembr3 = param.asInt();
  if (valuembr3 == 1) {
    relayControl_modbusRTU(2, 3, 1);
  } else {
    relayControl_modbusRTU(2, 3, 0);
  }
}
//============BUTTON ON/OFF mbr3===========//

//============BUTTON ON/OFF mbr4===========//
BLYNK_WRITE(Widget_btn_mbr4) {
  int valuembr4 = param.asInt();
  if (valuembr4 == 1) {
    relayControl_modbusRTU(2, 4, 1);
  } else {
    relayControl_modbusRTU(2, 4, 0);
  }
}
//============BUTTON ON/OFF mbr4===========//

//============BUTTON ON/OFF mbr5===========//
BLYNK_WRITE(Widget_btn_mbr5) {
  int valuembr5 = param.asInt();
  if (valuembr5 == 1) {
    relayControl_modbusRTU(2, 5, 1);
  } else {
    relayControl_modbusRTU(2, 5, 0);
  }
}
//============BUTTON ON/OFF mbr5===========//

//============BUTTON ON/OFF mbr6===========//
BLYNK_WRITE(Widget_btn_mbr6) {
  int valuembr6 = param.asInt();
  if (valuembr6 == 1) {
    relayControl_modbusRTU(2, 6, 1);
  } else {
    relayControl_modbusRTU(2, 6, 0);
  }
}
//============BUTTON ON/OFF mbr6===========//

//============BUTTON ON/OFF mbr7===========//
BLYNK_WRITE(Widget_btn_mbr7) {
  int valuembr7 = param.asInt();
  if (valuembr7 == 1) {
    relayControl_modbusRTU(2, 7, 1);
  } else {
    relayControl_modbusRTU(2, 7, 0);
  }
}
//============BUTTON ON/OFF mbr7===========//

//============BUTTON ON/OFF mbr8===========//
BLYNK_WRITE(Widget_btn_mbr8) {
  int valuembr8 = param.asInt();
  if (valuembr8 == 1) {
    relayControl_modbusRTU(2, 8, 1);
  } else {
    relayControl_modbusRTU(2, 8, 0);
  }
}
//============BUTTON ON/OFF mbr8===========//

//============BUTTON ON/OFF mbr9===========//
BLYNK_WRITE(Widget_btn_mbr9) {
  int valuembr9 = param.asInt();
  if (valuembr9 == 1) {
    relayControl_modbusRTU(2, 9, 1);
  } else {
    relayControl_modbusRTU(2, 9, 0);
  }
}
//============BUTTON ON/OFF mbr9===========//

//============BUTTON ON/OFF mbr10===========//
BLYNK_WRITE(Widget_btn_mbr10) {
  int valuembr10 = param.asInt();
  if (valuembr10 == 1) {
    relayControl_modbusRTU(2, 10, 1);
  } else {
    relayControl_modbusRTU(2, 10, 0);
  }
}
//============BUTTON ON/OFF mbr10===========//

//============BUTTON ON/OFF mbr11===========//
BLYNK_WRITE(Widget_btn_mbr11) {
  int valuembr11 = param.asInt();
  if (valuembr11 == 1) {
    relayControl_modbusRTU(2, 11, 1);
  } else {
    relayControl_modbusRTU(2, 11, 0);
  }
}
//============BUTTON ON/OFF mbr11===========//

//============Modbus-RTU Relay=============//

//============BUTTON ON/OFF mbr12===========//
BLYNK_WRITE(Widget_btn_mbr12) {
  int valuembr12 = param.asInt();
  if (valuembr12 == 1) {
    relayControl_modbusRTU(2, 12, 1);
  } else {
    relayControl_modbusRTU(2, 12, 0);
  }
}
//============BUTTON ON/OFF mbr12===========//

//========Check Status LED Widget===========//
void checkphysic_btn_state() {
  stateled1 = relayStatus_modbusRTU(2, 1);  //Check ON/OFF สวิตส์1
  if (stateled1 != prevStateled1) {
    if (stateled1 == 1) Blynk.virtualWrite(V1, 1);
    lcd.setCursor(0, 3);
    lcd.print("C1:");
    if (stateled1 == 1)
      lcd.print("ON ");

    if (stateled1 == 0) Blynk.virtualWrite(V1, 0);
    lcd.setCursor(0, 3);
    lcd.print("C1:");
    if (stateled1 == 0)
      lcd.print("OFF");
  }
  prevStateled1 = stateled1;

  stateled2 = relayStatus_modbusRTU(2, 2);  //Check ON/OFF สวิตส์2
  if (stateled2 != prevStateled2) {
    if (stateled2 == 1) Blynk.virtualWrite(V2, 1);
    lcd.setCursor(7, 3);
    lcd.print("C2:");
    if (stateled2 == 1)
      lcd.print("ON ");

    if (stateled2 == 0) Blynk.virtualWrite(V2, 0);
    lcd.setCursor(7, 3);
    lcd.print("C2:");
    if (stateled2 == 0)
      lcd.print("OFF");
  }
  prevStateled2 = stateled2;

  stateled3 = relayStatus_modbusRTU(2, 3);  //Check ON/OFF สวิตส์3
  if (stateled3 != prevStateled3) {
    if (stateled3 == 1) Blynk.virtualWrite(V3, 1);
    lcd.setCursor(14, 3);
    lcd.print("C3:");
    if (stateled3 == 1)
      lcd.print("ON ");
    if (stateled3 == 0) Blynk.virtualWrite(V3, 0);
    lcd.setCursor(14, 3);
    lcd.print("C3:");
    if (stateled3 == 0)
      lcd.print("OFF");
  }
  prevStateled3 = stateled3;

  stateled4 = relayStatus_modbusRTU(2, 4);  //Check ON/OFF สวิตส์4
  if (stateled4 != prevStateled4) {
    if (stateled4 == 1) Blynk.virtualWrite(V4, 1);
    if (stateled4 == 0) Blynk.virtualWrite(V4, 0);
  }
  prevStateled4 = stateled4;

  stateled5 = relayStatus_modbusRTU(2, 5);  //Check ON/OFF สวิตส์5
  if (stateled5 != prevStateled5) {
    if (stateled5 == 1) Blynk.virtualWrite(V5, 1);
    if (stateled5 == 0) Blynk.virtualWrite(V5, 0);
  }
  prevStateled5 = stateled5;

  stateled6 = relayStatus_modbusRTU(2, 6);  //Check ON/OFF สวิตส์6
  if (stateled6 != prevStateled6) {
    if (stateled6 == 1) Blynk.virtualWrite(V6, 1);
    if (stateled6 == 0) Blynk.virtualWrite(V6, 0);
  }
  prevStateled6 = stateled6;

  stateled7 = relayStatus_modbusRTU(2, 7);  //Check ON/OFF สวิตส์7
  if (stateled7 != prevStateled7) {
    if (stateled7 == 1) Blynk.virtualWrite(V7, 1);
    if (stateled7 == 0) Blynk.virtualWrite(V7, 0);
  }
  prevStateled7 = stateled7;

  stateled8 = relayStatus_modbusRTU(2, 8);  //Check ON/OFF สวิตส์8
  if (stateled8 != prevStateled8) {
    if (stateled8 == 1) Blynk.virtualWrite(V8, 1);
    if (stateled8 == 0) Blynk.virtualWrite(V8, 0);
  }
  prevStateled8 = stateled8;

  stateled9 = relayStatus_modbusRTU(2, 9);  //Check ON/OFF สวิตส์9
  if (stateled9 != prevStateled9) {
    if (stateled9 == 1) Blynk.virtualWrite(V9, 1);
    if (stateled9 == 0) Blynk.virtualWrite(V9, 0);
  }
  prevStateled9 = stateled9;

  stateled10 = relayStatus_modbusRTU(2, 10);  //Check ON/OFF สวิตส์10
  if (stateled10 != prevStateled10) {
    if (stateled10 == 1) Blynk.virtualWrite(V10, 1);
    if (stateled10 == 0) Blynk.virtualWrite(V10, 0);
  }
  prevStateled10 = stateled10;

  stateled11 = relayStatus_modbusRTU(2, 11);  //Check ON/OFF สวิตส์11
  if (stateled11 != prevStateled11) {
    if (stateled11 == 1) Blynk.virtualWrite(V11, 1);
    if (stateled11 == 0) Blynk.virtualWrite(V11, 0);
  }
  prevStateled11 = stateled11;

  stateled12 = relayStatus_modbusRTU(2, 12);  //Check ON/OFF สวิตส์6
  if (stateled12 != prevStateled12) {
    if (stateled12 == 1) Blynk.virtualWrite(V12, 1);
    if (stateled12 == 0) Blynk.virtualWrite(V12, 0);
  }
  prevStateled12 = stateled12;
}
//========Check Status LED Widget===========//

//==============Loop Function==============//
void loop() {
  display.display();
  display.clear();
  display.drawString(0, 1, "#ESP32 SMF01#");

  //Display temp on oled
  float BoardTemp;
  BoardTemp = Clock.getTemperature();
  display.drawString(0, 23, "Temp :" + String(BoardTemp) + " C");

  //Display date on oled
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
  display.drawString(0, 44, "Date " + String(currentDate));

  //LCD 2004
  lcd.setCursor(2, 0);
  lcd.print("FB/SMFTHAILAND");

  lcd.setCursor(2, 1);
  lcd.print("MCU TEMP:");
  lcd.print(BoardTemp);
  lcd.print(" C");

  lcd.setCursor(2, 2);
  lcd.print("DATE:");
  lcd.print(currentDate);

  BlynkEdgent.run();
  timer.run();
}
//==============Loop Function==============//
