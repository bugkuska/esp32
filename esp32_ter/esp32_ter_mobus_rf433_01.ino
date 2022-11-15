/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 15 Nov, 2022
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
    3.SimpleTimer https://github.com/jfturcot/SimpleTimer 
    4.NTPClient (download folder)
    5.LiquidCrystal_i2c (download folder) (Optional)
  *  ********************************************************************************************* 
    Create initials Blynk.Edgent file :
    File >> Example >> Blynk.Edgent >> Edgent_ESP32

    Sign-up and log-in to https://blynk.io/
    First, create device template for set of your hardware.
    Second, create data stream for receive your hardware that send to blynk.
    Third, create web and mobile dashboard (download and install mobile blynk application)
    Forth, acctivate the device
      - Manual activate
      - Wi-Fi provisioning
    Have fun....
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

//===========SW Valve1============//
#define relay1_valve1 18
//#define Widget_btn_valve1 V1
bool switchStatus1 = 0;  // 0 = manual,1=auto
int SoilLevelLimit1 = 0;
bool manualSwitch1 = 0;
//===========SW Valve1============//

//==========SimpleTimer===========//
#include <SimpleTimer.h>
SimpleTimer timer;
//==========SimpleTimer===========//

//==========Modbus-Master=========//
#include <ModbusMaster.h>
#define RXD1 26
#define TXD1 27
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 SoilMoisture01
//==========Modbus-Master=========//

//=============NTP================//
#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
//=============NTP================//

//==========Setup Function========//
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD1, TXD1);
  node1.begin(1, Serial2);
  delay(100);

  pinMode(relay1_valve1, OUTPUT);
  digitalWrite(relay1_valve1, HIGH);

  timer.setInterval(5000L, SoilMoisture01);  //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L, datetime);        //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  BlynkEdgent.begin();
}
//==========Setup Function========//

//========Blynk Connected=========//
BLYNK_CONNECTED() {
  if (Blynk.connected()) {
    Serial.println("Blynk Connected");
    Blynk.syncAll();
  }
}
//========Blynk Connected=========//

// Update switchStatus1 on SoilMoisture
BLYNK_WRITE(V5) {
  switchStatus1 = param.asInt();  // Get value as integer
}

// Update Soil setting
BLYNK_WRITE(V6) {
  SoilLevelLimit1 = param.asInt();  // Get value as integer
}

// Update manualSwitch1
BLYNK_WRITE(V1) {
  manualSwitch1 = param.asInt();
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(relay1_valve1, LOW);
  } else {
    digitalWrite(relay1_valve1, HIGH);
  }
}

//======SoilMoisture Sensor=======//
void SoilMoisture01() {
  uint8_t result;
  float soil_01 = (node1.getResponseBuffer(2) / 10.0f);

  Serial.println("Get Soil Moisture Data");
  result = node1.readHoldingRegisters(0x0000, 3);  // Read 2 registers starting at 1)
  if (result == node1.ku8MBSuccess) {
    Serial.print("Soil Moisture Sensor: ");
    Serial.println(node1.getResponseBuffer(2) / 10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V2, soil_01);

  if (switchStatus1) {
    // auto
    if (soil_01 < SoilLevelLimit1) {
      digitalWrite(relay1_valve1, LOW);
      Blynk.virtualWrite(V1, 1);
    } else {
      digitalWrite(relay1_valve1, HIGH);
      Blynk.virtualWrite(V1, 0);
    }
  } else {
    if (manualSwitch1) {
      digitalWrite(relay1_valve1, LOW);
    } else {
      digitalWrite(relay1_valve1, HIGH);
    }
    // manaul
  }
}
//======SoilMoisture Sensor=======//

//======Display NTP Date&Time=====//
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

  Blynk.virtualWrite(V3, currentTime);

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
  Blynk.virtualWrite(V4, currentDate);
}
//======Display NTP Date&Time=====//

//==========Loop Function=========//
void loop() {
  BlynkEdgent.run();
  timer.run();
}
//==========Loop Function=========//
