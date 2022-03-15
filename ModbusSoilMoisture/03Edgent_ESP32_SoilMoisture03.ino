//================Change Log================//
//Add Auto/Manual 
//Add Manual ON/OFF SW3, SW4
//================Change Log================//

//==============New Blynk IoT===============//
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION        "0.5.0" 
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

//==MCU Digital Pin Check blynk connected===//
#define Relay1_ledblynk           26
//==MCU Digital Pin Check blynk connected===//

//============Blynk Virtual Pin============//
//V1    Pump
//V2    ManualSW3
//V3    ManualSW4
//V6    soilMoisture
//V7    CurrentDate
//V8    CurrentTime
//V11   Auto&Manual Pump
//V12   Slider SoilMoisture
//============Blynk Virtual Pin============//

//===========ปุ่ม เปิด-ปิด sw2_pump============//
//V11 Auto&Manual Pump
//V12 Slider SoilMoisture
//Manual & Auto Switch Pump
#define Relay2_sw2_pump            25
#define Widget_Btn_sw2_pump        V1     
//Slider for set Soil limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int SoilLevelLimit1 = 0;
bool manualSwitch1 = 0;
//===========ปุ่ม เปิด-ปิด sw2_pump============//
     
#define Relay3_sw3            33
#define Widget_Btn_sw3        V2 
#define Relay4_sw4            32
#define Widget_Btn_sw4        V3 

//==================Modbus================//
#include <ModbusMaster.h>
///////// PIN /////////
#define MAX485_DE             18    //DE
#define MAX485_RE_NEG         19    //RE
#define RX2                   16    //RO,RX
#define TX2                   17    //DI,TX
// instantiate ModbusMaster object
ModbusMaster node1;                 //Modbus SoilMoisture
//==================Modbus================//

//====Modbus Pre & Post Transmission1====//
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
//====Modbus Pre & Post Transmission1====//

//=============Setup Function============//
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
  pinMode(Relay1_ledblynk,OUTPUT);        // ESP32 PIN gpio26
  pinMode(Relay2_sw2_pump,OUTPUT);        // ESP32 PIN GPIO25
  pinMode(Relay3_sw3,OUTPUT);             // ESP32 PIN GPIO32   
  pinMode(Relay4_sw4,OUTPUT);             // ESP32 PIN GPIO33         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_ledblynk,LOW);      // ESP32 PIN gpio26
  digitalWrite(Relay2_sw2_pump,LOW);      // ESP32 PIN GPIO25
  digitalWrite(Relay3_sw3,LOW);           // ESP32 PIN GPIO32   
  digitalWrite(Relay4_sw4,LOW);           // ESP32 PIN GPIO33    
  
  BlynkEdgent.begin();
  timer.setInterval(5000L, SoilMoisture01);
   timer.setInterval(5000L, datetime);
}
//=============Setup Function============//

// Update switchStatus1 on SoilMoisture
BLYNK_WRITE(V11)
{   
  switchStatus1 = param.asInt(); // Get value as integer
}

// Update Soil setting
BLYNK_WRITE(V12)
{   
  SoilLevelLimit1 = param.asInt(); // Get value as integer
}

// Update manualSwitch1
BLYNK_WRITE(V1)
{
  manualSwitch1 = param.asInt();
}


//========SoilMoisture Sensor==============//
void SoilMoisture01(){
  uint8_t result;  
  float soil_01 = (node1.getResponseBuffer(2)/10.0f);
  
  Serial.println("Get Soil Moisture Data");
  result = node1.readHoldingRegisters(0x0000, 3); // Read 2 registers starting at 1)
  if (result == node1.ku8MBSuccess)
  {
    Serial.print("Soil Moisture Sensor: ");
    Serial.println(node1.getResponseBuffer(2)/10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V6,soil_01);
  
  if(switchStatus1)
  {
    // auto
    if(soil_01 < SoilLevelLimit1)
    {
        digitalWrite(Relay2_sw2_pump, HIGH);  
        Blynk.virtualWrite(V1, 1); 
    }  
    else
    {
        digitalWrite(Relay2_sw2_pump, LOW);
        Blynk.virtualWrite(V1, 0); 
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay2_sw2_pump, HIGH);        
    }
    else
    {
        digitalWrite(Relay2_sw2_pump, LOW);
    }
    // manaul
  }
}
//========SoilMoisture Sensor==============//

//=========ON-OFF SW3=========//
 BLYNK_WRITE(Widget_Btn_sw3){
      int valuebtn3 = param.asInt();
      if(valuebtn3 == 1){
        digitalWrite(Relay3_sw3,HIGH);        
      }
       else{              
        digitalWrite(Relay3_sw3,LOW);        
     }
}
//=========ON-OFF SW3=========//

//=========ON-OFF SW4=========//
 BLYNK_WRITE(Widget_Btn_sw4){
      int valuebtn4 = param.asInt();
      if(valuebtn4 == 1){
        digitalWrite(Relay4_sw4,HIGH);        
      }
       else{              
        digitalWrite(Relay4_sw4,LOW);        
     }
}
//=========ON-OFF SW4=========//


//===========Display Date&Time===========//
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
  Blynk.virtualWrite(V7, dayStamp);
  
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  Blynk.virtualWrite(V8, timeStamp);  
  delay(1000);
}
//===========Display Date&Time===========//

//=============Blynk conneted============//
BLYNK_CONNECTED()
{
 if (Blynk.connected())
 {
    Serial.println("Blynk Connected");
    digitalWrite(Relay1_ledblynk, HIGH);  //ledpin for check blynk connected 
    Serial.println("Blynk connected");
    Blynk.syncAll();
 }
}
//=============Blynk conneted============//

//==============Loop Function============// 
void loop() {      
  BlynkEdgent.run();   
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
//==============Loop Function============// 
