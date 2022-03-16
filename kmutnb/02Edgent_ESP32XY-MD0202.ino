//==============New Blynk IoT===============//
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION        "0.2.0" 
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"
#include <SimpleTimer.h>
//==============New Blynk IoT===============//

//==================NTP=====================//
#include <NTPClient.h>
#include <WiFiUdp.h>
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
//==================NTP=====================//

//============Blynk Virtual Pin============//
//V1    SW1
//V2    SW2
//V3    SW3
//V4    Humi1
//V5    Temp1
//V6    CurrentDate
//V7    CurrentTime
//============Blynk Virtual Pin============//

//==============ปุ่ม เปิด-ปิด sw1==============//
#define Relay1_sw1                26
#define Widget_Btn_sw1            V1     
//==============ปุ่ม เปิด-ปิด sw1==============//
     
//=============ปุ่ม เปิด-ปิด sw2===============//
#define Relay2_sw2                25
#define Widget_Btn_sw2            V2          
//=============ปุ่ม เปิด-ปิด sw2===============//

//============ปุ่ม เปิด-ปิด sw3================//
#define Relay3_sw3                33
#define Widget_Btn_sw3            V3          
//============ปุ่ม เปิด-ปิด sw3================//

//==MCU Digital Pin Check blynk connected==//
#define ledblynk                      32
//==MCU Digital Pin Check blynk connected==//

//===================Modbus================//
#include <ModbusMaster.h>
///////// PIN /////////
#define MAX485_DE             18    //DE
#define MAX485_RE_NEG         19    //RE
#define RX2                   16    //RO,RX
#define TX2                   17    //DI,TX
// instantiate ModbusMaster object
ModbusMaster node1;
//==================Modbus=================//

//=====Modbus Pre & Post Transmission1=====//
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
//=====Modbus Pre & Post Transmission1=====//

//==============Setup Function=============//
void setup()
{
  //Modbus pinMode
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); //RX2=16,RO ,TX2=17, DI
 
  node1.begin(1, Serial2);
  node1.preTransmission(preTransmission1);
  node1.postTransmission(postTransmission1);

// Setup Pin Mode
  pinMode(Relay1_sw1,OUTPUT);         // ESP32 PIN gpio26
  pinMode(Relay2_sw2,OUTPUT);         // ESP32 PIN GPIO25
  pinMode(Relay3_sw3,OUTPUT);       // ESP32 PIN GPIO33   
  pinMode(ledblynk,OUTPUT);                 // ESP32 PIN GPIO15         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_sw1,LOW);       // ESP32 PIN gpio26
  digitalWrite(Relay2_sw2,LOW);       // ESP32 PIN GPIO25
  digitalWrite(Relay3_sw3,LOW);     // ESP32 PIN GPIO33     
  digitalWrite(ledblynk,LOW);              // ESP32 PIN GPIO15  
  
  BlynkEdgent.begin();
  timer.setInterval(10000L,datetime);
  timer.setInterval(5000L, xymd02);
}
//==============Setup Function=============//

//================WTR10-E Sensor===========//
void xymd02()
{
  uint8_t result1; 
  float temp1 = (node1.getResponseBuffer(0)/10.0f);
  float humi1 = (node1.getResponseBuffer(1)/10.0f);

  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2); // Read 2 registers starting at 1)
  if (result1 == node1.ku8MBSuccess)
  {
    Serial.print("Temp: ");
    Serial.println(node1.getResponseBuffer(0)/10.0f);
    Serial.print("Humi: ");
    Serial.println(node1.getResponseBuffer(1)/10.0f);
  }
  delay(1000);

  Blynk.virtualWrite(V4,humi1);
  Blynk.virtualWrite(V5,temp1);

}
//================WTR10-E Sensor===========//

//=============Display Date&Time===========//
void datetime() {

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200);

  while(!timeClient.update()) {
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
  Blynk.virtualWrite(V6, dayStamp);
  
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  Blynk.virtualWrite(V7, timeStamp);  
  delay(1000);
}
//=============Display Date&Time===========//

//==============Blynk conneted=============//
BLYNK_CONNECTED()
{
 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(ledblynk, HIGH);  //ledpin for check blynk connected 
    Serial.println("Blynk connected");
    Blynk.syncAll();
 }
}
//==============Blynk conneted=============//

//===============Loop Function=============// 
void loop() {      
  BlynkEdgent.run();   
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
//===============Loop Function=============// 

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
