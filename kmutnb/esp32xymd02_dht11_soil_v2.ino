//===============Blynk Virtual Pin==============//
//V1    SW1-pump
//V2    SW2-fan
//V3    SW3-led
//V4    XYMD02-Temp
//V5    XYMD02-Humi
//V6    DHT11-Temp
//V7    DHT11-Humi
//V8    SoilMoisture
//V9    CurrentDate
//V10   CurrentTime
//===============Blynk Virtual Pin==============//

//================New Blynk IoT=================//
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION        "0.2.0" 
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"
#include <SimpleTimer.h>
//================New Blynk IoT=================//

//==================DHT11=======================//
#include <DHT.h>                 
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
//==================DHT11=======================//

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

//==============ปุ่ม เปิด-ปิด sw1-pump==============//
#define Relay1_sw1                26
#define Widget_Btn_sw1            V1     
//==============ปุ่ม เปิด-ปิด sw1-pump==============//
     
//=============ปุ่ม เปิด-ปิด sw2-fan===============//
#define Relay2_sw2                25
#define Widget_Btn_sw2            V2          
//=============ปุ่ม เปิด-ปิด sw2-fan===============//

//============ปุ่ม เปิด-ปิด sw3-led================//
#define Relay3_sw3                33
#define Widget_Btn_sw3            V3          
//============ปุ่ม เปิด-ปิด sw3-led================//

//====MCU Digital Pin Check blynk connected====//
#define ledblynk                      32
//====MCU Digital Pin Check blynk connected====//

//=====================Modbus==================//
#include <ModbusMaster.h>
///////// PIN /////////
#define MAX485_DE             18    //DE
#define MAX485_RE_NEG         19    //RE
#define RX2                   16    //RO,RX
#define TX2                   17    //DI,TX
// instantiate ModbusMaster object
ModbusMaster node1; //XYMD-02
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
  timer.setInterval(5000L,xymd02); //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L,dht11); //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L,soil); //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(10000L,datetime);
}
//=================Setup Function==============//

//==================XYMD-02 Sensor=============//
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

  Blynk.virtualWrite(V4,temp1);
  Blynk.virtualWrite(V5,humi1);

}
//==================XYMD-02 Sensor=============//

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
}
//=================Soil Moisture===============//

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
