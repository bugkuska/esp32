//===============Blynk Virtual Pin==============//
//V1    SW1-pump
//V2    SW2-fan
//V3    SW3-led
//V4    XYMD02-Temp1
//V5    XYMD02-Humi1
//V6    DHT11-Temp
//V7    DHT11-Humi
//V8    SoilMoisture
//V9    CurrentDate
//V10   CurrentTime
//V11   Auto&Manual Pump
//V12   Slider SoilMoisture
//V13   XYMD02-Temp2
//V14   XYMD02-Humi2
//V15   XYMD02-Temp3
//V16   XYMD02-Humi3
//V17   Modbus_Soil01
//===============Blynk Virtual Pin==============//

//================New Blynk IoT=================//
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION        "0.2.3" 
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"
#include <SimpleTimer.h>
SimpleTimer timer;
//================New Blynk IoT=================//
  
//==================DHT11=======================//
#include <DHT.h>                 
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
//==================DHT11=======================//

//============Senddata2GGSheet=================//
#include <WiFi.h>
#include <HTTPClient.h>
const char* host = "script.google.com";
const char* httpsPort = "443";
String GAS_ID = "";
//============Senddata2GGSheet=================//

//=================Soil Moisture================//
#define INPUT_1 36
//=================Soil Moisture================//

//====================NTP=======================//
#include <NTPClient.h>
#include <WiFiUdp.h>
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
//====================NTP=======================//

//===========ปุ่ม เปิด-ปิด sw1_pump============//
//V11 Auto&Manual Pump
//V12 Slider SoilMoisture
//Manual & Auto Switch Pump
#define Relay1_sw1                26
#define Widget_Btn_sw1            V1    
//Slider for set Soil limit
bool switchStatus1 = 0; // 0 = manual,1=auto
int SoilLevelLimit1 = 0;
bool manualSwitch1 = 0;
//===========ปุ่ม เปิด-ปิด sw1_pump============//
     
//=============ปุ่ม เปิด-ปิด sw2-fan===============//
#define Relay2_sw2                25
#define Widget_Btn_sw2            V2          
//=============ปุ่ม เปิด-ปิด sw2-fan===============//

//============ปุ่ม เปิด-ปิด sw3-led================//
#define Relay3_sw3                33
#define Widget_Btn_sw3            V3          
//============ปุ่ม เปิด-ปิด sw3-led================//

//====MCU Digital Pin Check blynk connected====//
#define ledblynk                  32
//====MCU Digital Pin Check blynk connected====//

//=====================Modbus==================//
#include <ModbusMaster.h>
///////// PIN /////////
#define MAX485_DE             18    //DE
#define MAX485_RE_NEG         19    //RE
#define RX2                   16    //RO,RX
#define TX2                   17    //DI,TX
// instantiate ModbusMaster object
ModbusMaster node1; //XYMD-02-1
ModbusMaster node2; //XYMD-02-2
ModbusMaster node3; //XYMD-02-3
ModbusMaster node4; //Modbus_Soil01
//=====================Modbus==================//

//=======Modbus Pre & Post Transmission1=======//
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
//=======Modbus Pre & Post Transmission1=======//

//=======Modbus Pre & Post Transmission2=======//
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
//=======Modbus Pre & Post Transmission2=======//

//=======Modbus Pre & Post Transmission3=======//
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
//=======Modbus Pre & Post Transmission3=======//

//=======Modbus Pre & Post Transmission4=======//
void preTransmission4()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission4()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
//=======Modbus Pre & Post Transmission4=======//

//=================Setup Function==============//
void setup()
{
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  // use Serial (port 0); initialize debug serial comunication
  Serial.begin(9600);    
   // use Serial (port 1); initialize Modbus communication baud rate
   // Ref https://www.arduino.cc/reference/en/language/functions/communication/serial/begin/
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); //RX2=16,RO ,TX2=17, DI
    // communicate with Modbus slave ID 1 over Serial (port 1)
  node1.begin(1, Serial2);
  node1.preTransmission(preTransmission1);
  node1.postTransmission(postTransmission1);

    node2.begin(2, Serial2);
  node2.preTransmission(preTransmission2);
  node2.postTransmission(postTransmission2);

    node3.begin(3, Serial2);
  node3.preTransmission(preTransmission3);
  node3.postTransmission(postTransmission3);

    node4.begin(4, Serial2);
  node4.preTransmission(preTransmission4);
  node4.postTransmission(postTransmission4);
  //==============DHT11====================//
  dht.begin();
  //==============DHT11====================//

  // Setup Pin Mode
  pinMode(Relay1_sw1,OUTPUT);               // ESP32 PIN gpio26
  pinMode(Relay2_sw2,OUTPUT);               // ESP32 PIN GPIO25
  pinMode(Relay3_sw3,OUTPUT);               // ESP32 PIN GPIO33   
  pinMode(ledblynk,OUTPUT);                 // ESP32 PIN GPIO15         
  
  // Set Defult Relay Status
  digitalWrite(Relay1_sw1,LOW);             // ESP32 PIN gpio26
  digitalWrite(Relay2_sw2,LOW);             // ESP32 PIN GPIO25
  digitalWrite(Relay3_sw3,LOW);             // ESP32 PIN GPIO33     
  digitalWrite(ledblynk,LOW);               // ESP32 PIN GPIO15  
  
  BlynkEdgent.begin(); 
  timer.setInterval(5000L,xymd021); //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L,xymd022); //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L,xymd023); //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L,modbus_soil01); //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  
  timer.setInterval(5000L,dht11); //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L,soil); //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(10000L,sendData2GGSheet); //ส่งค่าเซ็นเซอร์ขึ้น google sheet ทุกๆ 10 วินาที
  timer.setInterval(10000L,datetime);
}
//=================Setup Function==============//

//==================XYMD-021 Sensor=============//
void xymd021()
{
  uint8_t result1; 
  float temp = (node1.getResponseBuffer(0)/10.0f);
  float humi = (node1.getResponseBuffer(1)/10.0f);

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

  Blynk.virtualWrite(V4,temp);
  Blynk.virtualWrite(V5,humi);
}
//==================XYMD-021 Sensor=============//

//==================XYMD-022 Sensor=============//
void xymd022()
{
  uint8_t result2; 
  float temp2 = (node2.getResponseBuffer(0)/10.0f);
  float humi2 = (node2.getResponseBuffer(1)/10.0f);

  Serial.println("Get XY-MD02 Data2:");
  result2 = node2.readInputRegisters(0x0001, 2); // Read 2 registers starting at 1)
  if (result2 == node2.ku8MBSuccess)
  {
    Serial.print("Temp2: ");
    Serial.println(node2.getResponseBuffer(0)/10.0f);
    Serial.print("Humi2: ");
    Serial.println(node2.getResponseBuffer(1)/10.0f);
  }
  delay(1000);

  Blynk.virtualWrite(V13,temp2);
  Blynk.virtualWrite(V14,humi2);
}
//==================XYMD-022 Sensor=============//

//==================XYMD-023 Sensor=============//
void xymd023()
{
  uint8_t result3; 
  float temp3 = (node3.getResponseBuffer(0)/10.0f);
  float humi3 = (node3.getResponseBuffer(1)/10.0f);

  Serial.println("Get XY-MD02 Data3:");
  result3 = node3.readInputRegisters(0x0001, 2); // Read 2 registers starting at 1)
  if (result3 == node3.ku8MBSuccess)
  {
    Serial.print("Temp3: ");
    Serial.println(node3.getResponseBuffer(0)/10.0f);
    Serial.print("Humi3: ");
    Serial.println(node3.getResponseBuffer(1)/10.0f);
  }
  delay(1000);

  Blynk.virtualWrite(V15,temp3);
  Blynk.virtualWrite(V16,humi3);
}
//==================XYMD-02 Sensor=============//

//===========Modbus SoilMoisture01=============//
void modbus_soil01(){
  uint8_t result4;  
  float soil_01 = (node4.getResponseBuffer(2)/10.0f);
  
  Serial.println("Get Soil Moisture Data4");
  result4 = node4.readHoldingRegisters(0x0000, 3); // Read 2 registers starting at 1)
  if (result4 == node4.ku8MBSuccess)
  {
    Serial.print("Soil Moisture Sensor4: ");
    Serial.println(node4.getResponseBuffer(2)/10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V17,soil_01);
}
//===========Modbus SoilMoisture01=============//

//=================DHT11=======================//
void dht11()
{
  float temp2 = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  float humi2 = dht.readHumidity();
  
  Serial.print("DHT11-Temperature:");         
  Serial.println(temp2);
  Serial.print("DHT11-Humidity:");
  Serial.println(humi2);
  delay(800);
  
  Blynk.virtualWrite(V6,temp2);
  Blynk.virtualWrite(V7,humi2);
}
//=================DHT11=======================//


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

//=================Soil Moisture===============//
void soil() {
  float moisture_percentage;
  int sensor_analog;
  sensor_analog = analogRead(INPUT_1);
  Serial.print("Law Soil data:");
  Serial.println(sensor_analog);
  moisture_percentage = map(sensor_analog, 0,4095.00, 0,100);   
  Serial.print("Moisture Percentage = ");
  Serial.print(moisture_percentage);
  Serial.print("%\n\n");
  delay(1000);

  Blynk.virtualWrite(V8,moisture_percentage);
  
  if(switchStatus1)
  {
    // auto
    if(moisture_percentage < SoilLevelLimit1)
    {
        digitalWrite(Relay1_sw1, HIGH);  
        Blynk.virtualWrite(V1, 1); 
    }  
    else
    {
        digitalWrite(Relay1_sw1, LOW);
        Blynk.virtualWrite(V1, 0); 
    }
  }
  else
  {
    if(manualSwitch1)
    {
        digitalWrite(Relay1_sw1, HIGH);        
    }
    else
    {
        digitalWrite(Relay1_sw1, LOW);
    }
    // manaul
  }
}
//=================Soil Moisture===============//

//===========SendData2GGSheet=================//
void sendData2GGSheet() {
  //XY-MD02
  uint8_t result1; 
  float xytemp1 = (node1.getResponseBuffer(0)/10.0f);
  float xyhumi1 = (node1.getResponseBuffer(1)/10.0f);
  //DHT11
  float dtemp2 = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  float dhumi2 = dht.readHumidity();
  //Soil Moisture
  float moisture_percentage;
  int sensor_analog;
  sensor_analog = analogRead(INPUT_1);
  moisture_percentage = map(sensor_analog, 0,4095.00, 0,100);   

  //XY-MD02-2
  uint8_t result2; 
  float xytemp2 = (node2.getResponseBuffer(0)/10.0f);
  float xyhumi2 = (node2.getResponseBuffer(1)/10.0f);
  //XY-MD02-3
  uint8_t result3; 
  float xytemp3 = (node3.getResponseBuffer(0)/10.0f);
  float xyhumi3 = (node3.getResponseBuffer(1)/10.0f);
  //Modbus SoilMoisture
  uint8_t result4;  
  float msoil_01 = (node4.getResponseBuffer(2)/10.0f);
  
  delay(1000);
  
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?temp=" + xytemp1 + "&humi=" + xyhumi1 + "&temp2=" + dtemp2 + "&humi2=" + dhumi2 + "&soil1=" + moisture_percentage + "&xytemp2=" + xytemp2  + "&xyhumi2=" + xyhumi2  + "&xytemp3=" + xytemp3  + "&xyhumi3=" + xyhumi3 + "&msoil1=" + msoil_01;
  //Serial.print(url);
  Serial.println("Posting Temperature and humidity data to Google Sheet");
  //---------------------------------------------------------------------
  //starts posting data to google sheet
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);
  //---------------------------------------------------------------------
  //getting response from google sheet
  String payload;
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Payload: " + payload);
  }
  //---------------------------------------------------------------------
  http.end();
}
//===========SendData2GGSheet=================//

//===============Display Date&Time=============//
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
  Blynk.virtualWrite(V9, dayStamp);
  
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  Blynk.virtualWrite(V10, timeStamp);  
  delay(1000);
}
//===============Display Date&Time=============//

//================Blynk conneted===============//
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
//================Blynk conneted===============//

//=================Loop Function===============// 
void loop() {      
  BlynkEdgent.run();   
  if (Blynk.connected())
    {
      Blynk.run();
    } 
      timer.run();
}
//=================Loop Function===============// 

/*
//=================Manual Switch1==============//
BLYNK_WRITE(Widget_Btn_sw1){
      int valuebtn1 = param.asInt();
      if(valuebtn1 == 1){
        digitalWrite(Relay1_sw1,HIGH);        
      }
       else{              
        digitalWrite(Relay1_sw1,LOW);        
     }
}
//=================Manual Switch1==============//
*/

//=================Manual Switch2==============//
BLYNK_WRITE(Widget_Btn_sw2){
      int valuebtn2 = param.asInt();
      if(valuebtn2 == 1){
        digitalWrite(Relay2_sw2,HIGH);        
      }
       else{              
        digitalWrite(Relay2_sw2,LOW);        
     }
}
//=================Manual Switch2==============//

//=================Manual Switch3==============//
BLYNK_WRITE(Widget_Btn_sw3){
      int valuebtn3 = param.asInt();
      if(valuebtn3 == 1){
        digitalWrite(Relay3_sw3,HIGH);        
      }
       else{              
        digitalWrite(Relay3_sw3,LOW);        
     }
}
//=================Manual Switch3==============//
