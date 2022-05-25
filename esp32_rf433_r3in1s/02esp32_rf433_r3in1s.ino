//=====================New Blynk IoT===================//
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION        "0.2.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"
#include <SimpleTimer.h>
//=====================New Blynk IoT===================//

//==========Pool size for Modbus Write command=========//
int8_t pool_size1;
//==========Pool size for Modbus Write command=========//

//=========================NTP=========================//
#include <NTPClient.h>
#include <WiFiUdp.h>
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
//=========================NTP=========================//

//==================MCU Digital Pin====================//
#define Relay1_ledblynk           26
#define Relay2                    25
#define Relay3                    33
#define Relay4                    32
//==================MCU Digital Pin====================//

//==Define Blynk Virtual Pin for Modbus Relay SW1-SW8==//
#define Widget_Btn_SW1 V1        //ปุ่ม เปิด-ปิด SW1
#define Widget_Btn_SW2 V2        //ปุ่ม เปิด-ปิด SW2
#define Widget_Btn_SW3 V3        //ปุ่ม เปิด-ปิด SW3
#define Widget_Btn_SW4 V4        //ปุ่ม เปิด-ปิด SW4
#define Widget_Btn_SW5 V5        //ปุ่ม เปิด-ปิด SW5
#define Widget_Btn_SW6 V6        //ปุ่ม เปิด-ปิด SW6
#define Widget_Btn_SW7 V7        //ปุ่ม เปิด-ปิด SW7
#define Widget_Btn_SW8 V8        //ปุ่ม เปิด-ปิด SW8
//==Define Blynk Virtual Pin for Modbus Relay SW1-SW8==//
//V9  Date
//V10 Time
//V11 3IN1 Humi2
//V12 3IN1 Temp2
//V13 3IN1 Light2
//V14 Soil Moisture3
//========================Modbus=======================//
#include <ModbusMaster.h>
///////// PIN /////////
#define MAX485_DE             18    //DE
#define MAX485_RE_NEG         19    //RE
#define RX2                   16    //RO,RX
#define TX2                   17    //DI,TX
// instantiate ModbusMaster object
ModbusMaster node1;                 //ModbusRTU Relay Slave ID1
ModbusMaster node2;                 //Modbus 3IN1 Temp,Humi and Light Slave ID2
ModbusMaster node3;                 //Modbus Soil Moisture Slave ID3
//========================Modbus=======================//

//============Modbus Pre & Post Transmission1==========//
void preTransmission1()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission1()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//============Modbus Pre & Post Transmission1==========//

//============Modbus Pre & Post Transmission2==========//
void preTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission2()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//============Modbus Pre & Post Transmission2==========//

//============Modbus Pre & Post Transmission3==========//
void preTransmission3()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission3()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//============Modbus Pre & Post Transmission3==========//

//===================Setup Function====================//
void setup()
{
  //Modbus pinMode
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); //RX2=16,RO ,TX2=17, DI

  //Slave ID1
  node1.begin(1, Serial2);
  node1.preTransmission(preTransmission1);
  node1.postTransmission(postTransmission1);
  //Slave ID2
  node2.begin(2, Serial2);
  node2.preTransmission(preTransmission2);
  node2.postTransmission(postTransmission2);
  //Slave ID3
  node3.begin(3, Serial2);
  node3.preTransmission(preTransmission3);
  node3.postTransmission(postTransmission3);

  // Setup Pin Mode
  pinMode(Relay1_ledblynk, OUTPUT);     // ESP32 PIN gpio26
  pinMode(Relay2, OUTPUT);              // ESP32 PIN GPIO25
  pinMode(Relay3, OUTPUT);              // ESP32 PIN GPIO33
  pinMode(Relay4, OUTPUT);              // ESP32 PIN GPIO32

  // Set Defult Relay Status
  digitalWrite(Relay1_ledblynk, LOW);   // ESP32 PIN gpio26
  digitalWrite(Relay2, LOW);            // ESP32 PIN GPIO25
  digitalWrite(Relay3, LOW);            // ESP32 PIN GPIO33
  digitalWrite(Relay4, LOW);            // ESP32 PIN GPIO32

  BlynkEdgent.begin();
  timer.setInterval(60000L, datetime);  //Sync time every 1 minute
  timer.setInterval(30000L, sensordata);  //send sensor data every 30 seconds
}
//===================Setup Function====================//

//====================Sensor data======================//
void sensordata() {
  //Slave ID 2
  uint8_t result2;
  float humi2 = (node2.getResponseBuffer(0) / 10.0f);
  float temp2 = (node2.getResponseBuffer(1) / 10.0f);
  float light2 = (node2.getResponseBuffer(2));
  float light_per2;

  Serial.println("GZWS Data1");
  result2 = node2.readHoldingRegisters(0x0000, 3); // Read 3 registers starting at 1)
  if (result2 == node2.ku8MBSuccess)
  {
    Serial.print("Humi2: ");
    Serial.println(node2.getResponseBuffer(0) / 10.0f);
    Serial.print("Temp2: ");
    Serial.println(node2.getResponseBuffer(1) / 10.0f);
    light_per2 = (light2 = node2.getResponseBuffer(2));
    light_per2 = map(light_per2, 0, 65535, 0, 100);
    Serial.print("Light2: ");
    Serial.println(light_per2);
  }

  //Slave ID 3
  uint8_t result3;
  float soil_03 = (node3.getResponseBuffer(2) / 10.0f);

  Serial.println("Get Soil Moisture Data");
  result3 = node3.readHoldingRegisters(0x0000, 3); // Read 2 registers starting at 1)
  if (result3 == node3.ku8MBSuccess)
  {
    Serial.print("Soil Moisture Sensor: ");
    Serial.println(node3.getResponseBuffer(2) / 10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V11, humi2);
  Blynk.virtualWrite(V12, temp2);
  Blynk.virtualWrite(V13, light_per2);
  Blynk.virtualWrite(V14, soil_03);
}
//====================Sensor data======================//

//==========BUTTON ON/OFF SW1 Modbus relay ch1=========//
BLYNK_WRITE(Widget_Btn_SW1) {
  int valueSW1 = param.asInt();
  if (valueSW1 == 1) {
    //Modbus command to ON/OFF Relay
    pool_size1 = node1.writeSingleRegister(0x01, 0x0100);
  }
  else {
    pool_size1 = node1.writeSingleRegister(0x01, 0x0200);
  }
}
//==========BUTTON ON/OFF SW1 Modbus relay ch1=========//

//==========BUTTON ON/OFF SW2 Modbus relay ch2=========//
BLYNK_WRITE(Widget_Btn_SW2) {
  int valueSW2 = param.asInt();
  if (valueSW2 == 1) {
    //Modbus command to ON/OFF Relay
    pool_size1 = node1.writeSingleRegister(0x02, 0x0100);
  }
  else {
    pool_size1 = node1.writeSingleRegister(0x02, 0x0200);
  }
}
//==========BUTTON ON/OFF SW2 Modbus relay ch2=========//

//==========BUTTON ON/OFF SW3 Modbus relay ch3=========//
BLYNK_WRITE(Widget_Btn_SW3) {
  int valueSW3 = param.asInt();
  if (valueSW3 == 1) {
    //Modbus command to ON/OFF Relay
    pool_size1 = node1.writeSingleRegister(0x03, 0x0100);
  }
  else {
    pool_size1 = node1.writeSingleRegister(0x03, 0x0200);
  }
}
//==========BUTTON ON/OFF SW3 Modbus relay ch3=========//

//==========BUTTON ON/OFF SW4 Modbus relay ch4=========//
BLYNK_WRITE(Widget_Btn_SW4) {
  int valueSW4 = param.asInt();
  if (valueSW4 == 1) {
    //Modbus command to ON/OFF Relay
    pool_size1 = node1.writeSingleRegister(0x04, 0x0100);
  }
  else {
    pool_size1 = node1.writeSingleRegister(0x04, 0x0200);
  }
}
//==========BUTTON ON/OFF SW4 Modbus relay ch4=========//

//==========BUTTON ON/OFF SW5 Modbus relay ch5=========//
BLYNK_WRITE(Widget_Btn_SW5) {
  int valueSW5 = param.asInt();
  if (valueSW5 == 1) {
    //Modbus command to ON/OFF Relay
    pool_size1 = node1.writeSingleRegister(0x05, 0x0100);
  }
  else {
    pool_size1 = node1.writeSingleRegister(0x05, 0x0200);
  }
}
//==========BUTTON ON/OFF SW5 Modbus relay ch5=========//

//==========BUTTON ON/OFF SW6 Modbus relay ch6=========//
BLYNK_WRITE(Widget_Btn_SW6) {
  int valueSW6 = param.asInt();
  if (valueSW6 == 1) {
    //Modbus command to ON/OFF Relay
    pool_size1 = node1.writeSingleRegister(0x06, 0x0100);
  }
  else {
    pool_size1 = node1.writeSingleRegister(0x06, 0x0200);
  }
}
//==========BUTTON ON/OFF SW6 Modbus relay ch6=========//

//==========BUTTON ON/OFF SW7 Modbus relay ch7=========//
BLYNK_WRITE(Widget_Btn_SW7) {
  int valueSW7 = param.asInt();
  if (valueSW7 == 1) {
    //Modbus command to ON/OFF Relay
    pool_size1 = node1.writeSingleRegister(0x07, 0x0100);
  }
  else {
    pool_size1 = node1.writeSingleRegister(0x07, 0x0200);
  }
}
//==========BUTTON ON/OFF SW7 Modbus relay ch7=========//

//==========BUTTON ON/OFF SW8 Modbus relay ch8=========//
BLYNK_WRITE(Widget_Btn_SW8) {
  int valueSW8 = param.asInt();
  if (valueSW8 == 1) {
    //Modbus command to ON/OFF Relay
    pool_size1 = node1.writeSingleRegister(0x08, 0x0100);
  }
  else {
    pool_size1 = node1.writeSingleRegister(0x08, 0x0200);
  }
}
//==========BUTTON ON/OFF SW8 Modbus relay ch8=========//

//=================Display NTP Date&Time===============//
void datetime() {

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200);

  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  Blynk.virtualWrite(V9, dayStamp);

  // Extract time
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  Blynk.virtualWrite(V10, timeStamp);
  delay(1000);
}
//=================Display NTP Date&Time===============//

//====================Blynk conneted===================//
BLYNK_CONNECTED()
{
  if (Blynk.connected())
  {
    digitalWrite(Relay1_ledblynk, HIGH);  //ledpin for check blynk connected
    Serial.println("Blynk connected");
    Blynk.syncAll();
  }
}
//====================Blynk conneted===================//

//====================Loop Function====================//
void loop() {
  BlynkEdgent.run();
  timer.run();
}
//====================Loop Function====================//
