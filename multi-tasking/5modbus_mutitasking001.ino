/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 6 Mar, 2023
    Update On: 13 June, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    1. Five Modbus RTU 
      Slave ID1 : WTR10-E Temperature & Humidity
      Slave ID6 : 12IN12OUT Multi-Function Modbus RTU Relay
      Slave ID3 : WindSpeed Sensor
      Slave ID4 : 3IN1 GNSw1 Temperature, Humidity and Lux Sensor
      Slave ID5 : CO2 
    2. Multi-tasking Programing
    3. Display data & control via Nextion TFT Touch Screen
    4. Display data & control via Blynk webdashboard & blynk mobile application
    5. Data logger to Google sheet
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1. ITEADLIB_Arduino_Nextion https://github.com/itead/ITEADLIB_Arduino_Nextion/archive/refs/heads/master.zip
    2. Blynk Library https://github.com/blynkkk/blynk-library/archive/refs/heads/master.zip
    3. Modbus Master https://github.com/4-20ma/ModbusMaster/archive/refs/heads/master.zip
    4. Simple Timer https://github.com/jfturcot/SimpleTimer/archive/refs/heads/master.zip
    5. Wi-Fi Manager https://github.com/tzapu/WiFiManager
    6. ArduinoJson Ver 5.13.4   //https://github.com/bblanchon/ArduinoJson
    7. NTPClient https://github.com/arduino-libraries/NTPClient/archive/refs/heads/master.zip
  *  *********************************************************************************************
    Wiring Nextion to ESP32
    5V >> 5V
    GND >> GND
    RX >> TX0
    TX >> RX0, ถ้าแก้ไขไฟล์ NexConfig.h ใน Library ITEADLIB_Arduino_Nextion สามารถเปลี่ยนมาใช้ RX0 ได้
  *Note : ระหว่าง upload source-code ให้ถอดสาย RX2 ออกจากบอร์ด ESP32 พอ upload source-code ค่อยต่อเข้าไปใหม่
  2. Download and install Modbus-Master, Blynk and Simple-Timer
    Hoo ya, have fun....
 *************************************************************************************************/
/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID ""    //Template ID from Blynk.console>device info
#define BLYNK_TEMPLATE_NAME ""  //Template Name from Blynk.console>device info
#define BLYNK_AUTH_TOKEN ""     //Auth Token from Blynk.console>device info
#define BLYNK_FIRMWARE_VERSION "0.1.0"
bool fetch_blynk_state = true;  //true or false
//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = BLYNK_AUTH_TOKEN;
//================Wi-Fi Manager================//
#include <FS.h>  //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
//Callback notifying us of the need to save config
bool shouldSaveConfig = false;
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
//================Wi-Fi Manager================//
//==============Check Wi-Fi State==============//
int wifiFlag = 0;
#define wifiLed 2  //D2
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//==============Check Wi-Fi State==============//
//==============Senddata2GGSheet===============//
#include <WiFi.h>
#include <HTTPClient.h>
const char *host = "script.google.com";
const char *httpsPort = "443";
String GAS_ID = "";  //Google Script id from deploy app
//==============Senddata2GGSheet===============//
//=================SimpleTimer==================//
#include <SimpleTimer.h>
SimpleTimer timer;
//=================SimpleTimer==================//
//====================NTP=======================//
#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
//====================NTP=======================//
//=========Nextion Library and Object===========//
#include "Nextion.h"  //Nextion libraries
// (page id, component id, component name)
//Page id 3 DualState Button
//NexButton bt0 = NexButton(3, 2, "bt0");
//NexButton bt1 = NexButton(3, 3, "bt1");

//Page id 0 Home Screen2
NexText currentDate = NexText(0, 4, "currentDate");
NexText currentTime = NexText(0, 3, "currentTime");
//Page id 3 Text WTR10-E
NexText textemp1 = NexText(3, 2, "textemp1");
NexText texhumi1 = NexText(3, 3, "texhumi1");
//Page id 4 Progress BarWTR10-E
NexText textemp2 = NexText(4, 2, "textemp2");
NexText texhumi2 = NexText(4, 3, "texhumi2");
NexProgressBar pgtemp1 = NexProgressBar(4, 4, "pgtemp1");
NexProgressBar pghumi1 = NexProgressBar(4, 5, "pghumi1");
//Page id 5 Guage WTR10-E
NexGauge guagetemp1 = NexGauge(5, 2, "guagetemp1");
NexGauge guagehumi1 = NexGauge(5, 3, "guagehumi1");
//Page id 6 Modbus Relay
NexButton bt2 = NexButton(6, 2, "bt2");
NexButton bt3 = NexButton(6, 3, "bt3");
NexButton bt4 = NexButton(6, 4, "bt4");
NexButton bt5 = NexButton(6, 5, "bt5");
NexButton bt6 = NexButton(6, 6, "bt6");
NexButton bt7 = NexButton(6, 7, "bt7");
NexButton bt8 = NexButton(6, 8, "bt8");
NexButton bt9 = NexButton(6, 9, "bt9");
//Page id 7 Windspeed Sensor
NexGauge guagewind = NexGauge(7, 5, "guagewind");
NexText txtwind = NexText(7, 6, "txtwind");
NexText txtwind1 = NexText(7, 7, "txtwind1");
//Page id 8 GNSW-1 3IN1 Temp, Humi & Lux
NexText texthumi3 = NexText(8, 7, "texthumi3");
NexText textemp3 = NexText(8, 8, "textemp3");
NexText textlux3 = NexText(8, 9, "textlux3");
NexGauge guagehumi3 = NexGauge(8, 5, "guagehumi3");
NexGauge guagetemp3 = NexGauge(8, 6, "guagetemp3");
//Page 9 CO2
NexText textco2 = NexText(9, 4, "textco2");
//=========Nextion Library and Object===========//

//==========Declare variable global=============//
//bool statusbt0 = false;
//bool statusbt1 = false;
//Modbus Relay
bool statusbt2 = false;
bool statusbt3 = false;
bool statusbt4 = false;
bool statusbt5 = false;
bool statusbt6 = false;
bool statusbt7 = false;
bool statusbt8 = false;
bool statusbt9 = false;
//==========Declare variable global=============//
//===Register objects to the touch event list===//
NexTouch *nex_listen_list[] = {
  //2CH Relay
  //&bt0,
  //&bt1,
  //Modbus Relay
  &bt2,
  &bt3,
  &bt4,
  &bt5,
  &bt6,
  &bt7,
  &bt8,
  &bt9,
  NULL
};
//===Register objects to the touch event list===//

//==========Define IO connect to relay==========//
//#define sw1 33
//#define sw2 32
//#define sw1 18
//#define sw2 19
//=================Modbus-Master================//
#include <ModbusMaster.h>
#define RXD2 16  //RXD2 Serial2 Sensor Data
#define TXD2 17  //TXD2 Serial2 Sensor Data
#define RXD1 26  //RXD1 Serial1 Modbus Relay
#define TXD1 27  //TXD1 Serial1 Modbus Relay
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 WTR10-E
float humi1 = 0.0f;
float temp1 = 0.0f;
// instantiate ModbusMaster object
ModbusMaster node2;  //Serial2 Slave ID6 Modbus RTU Relay
int8_t pool_size1;   //Pool size for Modbus Write command
ModbusMaster node3;  //Serial2 Slave ID3 Modbus WindSpeed Sensor
ModbusMaster node4;  //Serial2 Slave ID4 Modbus 3IN1 Temp,Humi & Lux
ModbusMaster node5;  //Serial2 Slave ID5 Modbus CO2
//=================Modbus-Master================//
//==============Virtual Task Delay==============//
const TickType_t xDelay1000ms = pdMS_TO_TICKS(1000);    //Virtual task delay for TaskOne (Blynk and timer handle)
const TickType_t xDelay2000ms = pdMS_TO_TICKS(2000);    //Virtual task delay for TaskTwo (Send WTR10-E to Nextion)
const TickType_t xDelay5000ms = pdMS_TO_TICKS(5000);    //Virtual task delay for TaskThree (Send WTR10-E to Blynk)
const TickType_t xDelay10000ms = pdMS_TO_TICKS(10000);  //Virtual task delay for TaskFour (Send WTR10-E to Google Sheet)
//==============Virtual Task Delay==============//
//================Modbus Relay==================//
//=================ON,OFF MRelay1===============//
void bt2PushCallback(void *ptr) {
  if (statusbt2 == false) {
    pool_size1 = node2.writeSingleRegister(0x00, 0x0100);
    statusbt2 = true;
  } else if (statusbt2 == true) {
    pool_size1 = node2.writeSingleRegister(0x00, 0x0200);
    statusbt2 = false;
  }
}
//=================ON,OFF MRelay1===============//
//=================ON,OFF MRelay2===============//
void bt3PushCallback(void *ptr) {
  if (statusbt3 == false) {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0100);
    statusbt3 = true;
  } else if (statusbt3 == true) {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0200);
    statusbt3 = false;
  }
}
//=================ON,OFF MRelay2===============//
//=================ON,OFF MRelay3===============//
void bt4PushCallback(void *ptr) {
  if (statusbt4 == false) {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0100);
    statusbt4 = true;
  } else if (statusbt4 == true) {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0200);
    statusbt4 = false;
  }
}
//=================ON,OFF MRelay3===============//
//=================ON,OFF MRelay4===============//
void bt5PushCallback(void *ptr) {
  if (statusbt5 == false) {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0100);
    statusbt5 = true;
  } else if (statusbt5 == true) {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0200);
    statusbt5 = false;
  }
}
//=================ON,OFF MRelay4===============//
//=================ON,OFF MRelay5===============//
void bt6PushCallback(void *ptr) {
  if (statusbt6 == false) {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0100);
    statusbt6 = true;
  } else if (statusbt6 == true) {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0200);
    statusbt6 = false;
  }
}
//=================ON,OFF MRelay5===============//
//=================ON,OFF MRelay6===============//
void bt7PushCallback(void *ptr) {
  if (statusbt7 == false) {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0100);
    statusbt7 = true;
  } else if (statusbt7 == true) {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0200);
    statusbt7 = false;
  }
}
//=================ON,OFF MRelay6===============//
//=================ON,OFF MRelay7===============//
void bt8PushCallback(void *ptr) {
  if (statusbt8 == false) {
    pool_size1 = node2.writeSingleRegister(0x06, 0x0100);
    statusbt8 = true;
  } else if (statusbt8 == true) {
    pool_size1 = node2.writeSingleRegister(0x06, 0x0200);
    statusbt8 = false;
  }
}
//=================ON,OFF MRelay7===============//
//=================ON,OFF MRelay8===============//
void bt9PushCallback(void *ptr) {
  if (statusbt9 == false) {
    pool_size1 = node2.writeSingleRegister(0x07, 0x0100);
    statusbt9 = true;
  } else if (statusbt9 == true) {
    pool_size1 = node2.writeSingleRegister(0x07, 0x0200);
    statusbt9 = false;
  }
}
//=================ON,OFF MRelay8===============//
//================Modbus Relay==================//
//==============initWi-Fi Manager===============//
void initWiFiManager() {
  //read configuration from FS json
  Serial.println("mounting FS...");  //แสดงข้อความใน Serial Monitor
  if (SPIFFS.begin(true)) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
        } else {
          Serial.println("failed to load json config");  //แสดงข้อความใน Serial Monitor
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");  //แสดงข้อความใน Serial Monitor
  }
  //end read
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  /*
  for (int i = 5; i > -1; i--) {  // นับเวลาถอยหลัง 5 วินาทีก่อนกดปุ่ม AP Config
    //digitalWrite(ledbb, HIGH);
    delay(500);
    //digitalWrite(ledbb, LOW);
    // delay(500);
    Serial.print (String(i) + " ");//แสดงข้อความใน Serial Monitor 
  }
  */
  /*
  if (digitalRead(AP_Config) == LOW) {
   // Serial.println("Button Pressed");//แสดงข้อความใน Serial Monitor
    // wifiManager.resetSettings();//ให้ล้างค่า SSID และ Password ที่เคยบันทึกไว้
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //load the flash-saved configs
    esp_wifi_init(&cfg); //initiate and allocate wifi resources (does not matter if connection fails)
    delay(2000); //wait a bit
    if (esp_wifi_restore() != ESP_OK)
    {
      Serial.println("WiFi is not initialized by esp_wifi_init ");
    } else {
      Serial.println("WiFi Configurations Cleared!");
    }
    //continue
    //delay(1000);
    //esp_restart(); //just my reset configs routine...
  }
*/
  wifiManager.setTimeout(120);
  //ใช้ได้ 2 กรณี
  //1. เมื่อกดปุ่มเพื่อ Config ค่า AP แล้ว จะขึ้นชื่อ AP ที่เราตั้งขึ้น
  //   ช่วงนี้ให้เราทำการตั้งค่า SSID+Password หรืออื่นๆทั้งหมด ภายใน 60 วินาที ก่อน AP จะหมดเวลา
  //   ไม่เช่นนั้น เมื่อครบเวลา 60 วินาที MCU จะ Reset เริ่มต้นใหม่ ให้เราตั้งค่าอีกครั้งภายใน 60 วินาที
  //2. ช่วงไฟดับ Modem router + MCU จะดับทั้งคู่ และเมื่อมีไฟมา ทั้งคู่ก็เริ่มทำงานเช่นกัน
  //   โดยปกติ Modem router จะ Boot ช้ากว่า  MCU ทำให้ MCU กลับไปเป็น AP รอให้เราตั้งค่าใหม่
  //   ดังนั้น AP จะรอเวลาให้เราตั้งค่า 60 วินาที ถ้าไม่มีการตั้งค่าใดๆ เมื่อครบ 60 วินาที MCU จะ Reset อีกครั้ง
  //   ถ้า Modem router  Boot และใช้งานได้ภายใน 60 วินาที และหลังจากที่ MCU Resset และเริ่มทำงานใหม่
  //   ก็จะสามารถเชื่อมต่อกับ  Modem router ที่ Boot และใช้งานได้แล้ว  ได้  ระบบจะทำงานปกติ
  if (!wifiManager.autoConnect("Config-Node1", "0814111142")) {  //ชื่อ Soft AP ที่เราต้องเชื่อมต่อเพื่อเข้าหน้าตั้งค่าการเชื่อมต่อ Wi-Fi
    Serial.println("failed to connect and hit timeout");         //แสดงข้อความใน Serial Monitor
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();  //แก้ เดิม ESP.reset(); ใน Esp8266
    //delay(5000);
  }
  Serial.println("Connected.......OK!)");  //แสดงข้อความใน Serial Monitor
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");  //แสดงข้อความใน Serial Monitor
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  Serial.println("local ip");  //แสดงข้อความใน Serial Monitor
  delay(100);
  Serial.println(WiFi.localIP());  //แสดงข้อความใน Serial Monitor
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
}
//==============initWi-Fi Manager===============//

//================Setup Function================//
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  //Modbus Sensor Data
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);  //Modbus RTU Relay
  node1.begin(1, Serial2);                      //Slave ID1 WTR10-E
  node2.begin(6, Serial2);                      //Slave ID6 Modbus RTU Relay
  node3.begin(3, Serial2);                      //Slave ID3 Modbus WindSpeed Sensor
  node4.begin(4, Serial2);                      //Slave ID4 Modbus 3IN1 Temp, Humi & Lux
  node5.begin(5, Serial2);                      //Slave ID5 Modbus CO2
  //Initialize Nextion Library
  nexInit();
  // Register the push/pop event callback function
  bt2.attachPush(bt2PushCallback, &bt2);
  bt3.attachPush(bt3PushCallback, &bt3);
  bt4.attachPush(bt4PushCallback, &bt4);
  bt5.attachPush(bt5PushCallback, &bt5);
  bt6.attachPush(bt6PushCallback, &bt6);
  bt7.attachPush(bt7PushCallback, &bt7);
  bt8.attachPush(bt8PushCallback, &bt8);
  bt9.attachPush(bt9PushCallback, &bt9);
  // Set IO pinMode for relay
  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, LOW);
  delay(2000);
  //Wi-Fi Manager
  initWiFiManager();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  //Set timer Interval
  timer.setInterval(30000L, checkBlynkStatus);
  timer.setInterval(5000L, DateTime);
  //Begin blynk configure
  Blynk.config(auth);
  delay(1000);
  if (!fetch_blynk_state) {
    //Blynk.virtualWrite(V3, HIGH);
    //Blynk.virtualWrite(V4, HIGH);
  }
  //สร้าง Task สำหรับจัดการงานหลายงาน
  //taskOne สำหรับอ่านข้อมูล Sensor ตัวที่ 1 และส่งค่าไปแสดงผลบนจอ Nextion 
  xTaskCreate(
    taskOne,   /* Task function. */
    "TaskOne", /* String with name of task. */
    10000,     /* Stack size in words. */
    NULL,      /* Parameter passed as input of the task */
    1,         /* Priority of the task. */
    NULL);     /* Task handle. */
  //taskTwo สำหรับอ่านข้อมูล Sensor ตัวที่ 1 และส่งค่าไปแสดงผลบน Blynk
  xTaskCreate(
    taskTwo,   /* Task function. */
    "TaskTwo", /* String with name of task. */
    10000,     /* Stack size in words. */
    NULL,      /* Parameter passed as input of the task */
    1,         /* Priority of the task. */
    NULL);     /* Task handle. */
  //taskThree สำหรับอ่านข้อมูล Sensor ตัวที่ 1 และส่งค่าไปเก็บไว้บน Google Sheet 
  xTaskCreate(
    taskThree,   /* Task function. */
    "TaskThree", /* String with name of task. */
    10000,       /* Stack size in words. */
    NULL,        /* Parameter passed as input of the task */
    1,           /* Priority of the task. */
    NULL);
  //taskFour จัดการการเชื่อมต่อไปที่ Blynk, Timer 
  xTaskCreate(
    taskFour,   /* Task function. */
    "TaskFour", /* String with name of task. */
    10000,      /* Stack size in words. */
    NULL,       /* Parameter passed as input of the task */
    1,          /* Priority of the task. */
    NULL);
  //taskFive NTP Date Time
  //xTaskCreate(
  //  taskFive,   /* Task function. */
  //  "TaskFive", /* String with name of task. */
  //  10000,      /* Stack size in words. */
  //  NULL,       /* Parameter passed as input of the task */
  //  1,          /* Priority of the task. */
  //  NULL);
}
//================Setup Function================//

//==================taskTwo=====================//
void taskOne(void *parameter) {
  Serial.println(String("taskOne Sensor to Nextion"));
  while (true) {
    //Slave ID1
    uint8_t result1;
    float temp1 = (node1.getResponseBuffer(0) / 10.0f);
    float humi1 = (node1.getResponseBuffer(1) / 10.0f);
    Serial.println("Get WTR10-E Data:");
    //result1 = node1.readInputRegisters(0x0001, 2);  //Function 04, Read 2 registers starting at 2)
    result1 = node1.readHoldingRegisters(0x0000, 2);  // Read 2 registers starting at 1)
    if (result1 == node1.ku8MBSuccess) {
      Serial.print("Temp1: ");
      Serial.println(node1.getResponseBuffer(0) / 10.0f);
      Serial.print("Humi1: ");
      Serial.println(node1.getResponseBuffer(1) / 10.0f);
    }
    //Page 3 Text WTR10-E
    String command1 = "textemp1.txt=\"" + String(temp1) + "\"";
    Serial.print(command1);
    endNextionCommand();

    String command2 = "textemp1.txt=\"" + String(temp1) + "\"";
    Serial.print(command2);
    endNextionCommand();

    String command3 = "texhumi1.txt=\"" + String(humi1) + "\"";
    Serial.print(command3);
    endNextionCommand();

    //Page 4 Progress Bar WTR10-E
    String command4 = "textemp2.txt=\"" + String(temp1) + "\"";
    Serial.print(command4);
    endNextionCommand();

    String command5 = "textemp2.txt=\"" + String(temp1) + "\"";
    Serial.print(command5);
    endNextionCommand();

    String command6 = "texhumi2.txt=\"" + String(humi1) + "\"";
    Serial.print(command6);
    endNextionCommand();

    pgtemp1.setValue(temp1);
    Serial.print("pgtemp1.val=");
    Serial.print(temp1);
    endNextionCommand();

    pghumi1.setValue(humi1);
    Serial.print("pghumi1.val=");
    Serial.print(humi1);
    endNextionCommand();

    //Page 5 Guage WTR10-E
    int val1 = map(temp1, 0, 100, 0, 223);
    Serial.print("guagetemp1.val=");  //Send the object tag
    Serial.print(val1);               //Send the value
    endNextionCommand();

    int val2 = map(humi1, 0, 100, 0, 223);
    Serial.print("guagehumi1.val=");  //Send the object tag
    Serial.print(val2);               //Send the value
    endNextionCommand();

    delay(1000);
    //Slave ID3 Windspeed
    int windspeed = (node3.getResponseBuffer(0) / 10);
    uint8_t result2;
    Serial.println("Get WindSpeed data");
    result2 = node3.readHoldingRegisters(0x0000, 1);  // Read 2 registers starting at 1)
    if (result2 == node3.ku8MBSuccess) {
      Serial.print("Winspeed: ");
      Serial.println(node3.getResponseBuffer(0) / 10);
    }
    String command7 = "txtwind.txt=\"" + String(windspeed) + "\"";
    Serial.print(command7);
    endNextionCommand();

    String command14 = "txtwind.txt=\"" + String(windspeed) + "\"";
    Serial.print(command14);
    endNextionCommand();

    String command8 = "txtwind1.txt=\"" + String(windspeed) + "\"";
    Serial.print(command8);
    endNextionCommand();

    int val3 = map(windspeed, 0, 30, 0, 180);
    Serial.print("guagewind.val=");  //Send the object tag
    Serial.print(val3);              //Send the value
    endNextionCommand();
    delay(1000);

    //Slave ID4
    uint8_t result3;
    float humi3 = (node4.getResponseBuffer(0) / 10.0f);
    float temp3 = (node4.getResponseBuffer(1) / 10.0f);
    int lux3 = (node4.getResponseBuffer(2));
    result3 = node4.readHoldingRegisters(0x0000, 3);  // Read 2 registers starting at 1)
    if (result3 == node4.ku8MBSuccess) {
      Serial.print("3IN1Temp1: ");
      Serial.println(node1.getResponseBuffer(0) / 10.0f);
      Serial.print("3IN1Humi1: ");
      Serial.println(node1.getResponseBuffer(1) / 10.0f);
      Serial.print("3IN1Lux1: ");
      Serial.println(node4.getResponseBuffer(2));
    }
    String command9 = "texthumi3.txt=\"" + String(humi3) + "\"";
    Serial.print(command9);
    endNextionCommand();

    String command10 = "textemp3.txt=\"" + String(temp3) + "\"";
    Serial.print(command10);
    endNextionCommand();

    String command11 = "textlux3.txt=\"" + String(lux3) + "\"";
    Serial.print(command11);
    endNextionCommand();

    int val4 = map(humi3, 0, 100, 0, 205);
    Serial.print("guagehumi3.val=");  //Send the object tag
    Serial.print(val4);               //Send the value
    endNextionCommand();

    int val5 = map(temp3, 0, 100, 0, 205);
    Serial.print("guagetemp3.val=");  //Send the object tag
    Serial.print(val5);               //Send the value
    endNextionCommand();
    delay(1000);
    //Slave ID5
    uint8_t result4;
    float CO2 = (node5.getResponseBuffer(5));
    Serial.println("Get CO2");
    result4 = node5.readHoldingRegisters(0x0000, 7);  // Read 7 registers starting at 0)
    if (result4 == node5.ku8MBSuccess) {
      Serial.print("CO2: ");
      Serial.println(node5.getResponseBuffer(5));
    }
    String command12 = "textco2.txt=\"" + String(CO2) + "\"";
    Serial.print(command12);
    endNextionCommand();

    String command13 = "textco2.txt=\"" + String(CO2) + "\"";
    Serial.print(command13);
    endNextionCommand();
    delay(1000);
  }
  vTaskDelay(xDelay2000ms);
}
//==================taskOne=====================//
//===================taskTwo====================//
void taskTwo(void *parameter) {
  Serial.println(String("taskTwo Sensor Data to Blynk"));
  while (true) {
    uint8_t result1;
    float temp1 = (node1.getResponseBuffer(0) / 10.0f);
    float humi1 = (node1.getResponseBuffer(1) / 10.0f);
    Serial.println("Get WTR10-E Data:");
    //result1 = node1.readInputRegisters(0x0001, 2);  //Function 04, Read 2 registers starting at 2)
    result1 = node1.readHoldingRegisters(0x0000, 2);  // Read 2 registers starting at 1)
    if (result1 == node1.ku8MBSuccess) {
      Serial.print("Temp1: ");
      Serial.println(node1.getResponseBuffer(0) / 10.0f);
      Serial.print("Humi1: ");
      Serial.println(node1.getResponseBuffer(1) / 10.0f);
    }
    delay(2000);
    Blynk.virtualWrite(V1, temp1);
    Blynk.virtualWrite(V2, humi1);
  }
  vTaskDelay(xDelay5000ms);
}
//===================taskTwo====================//
//==================taskThree===================//
void taskThree(void *parameter) {
  Serial.println(String("taskThree Sensor Data to Google Sheet"));
  while (true) {
    uint8_t result1;
    float temp1 = (node1.getResponseBuffer(0) / 10.0f);
    float humi1 = (node1.getResponseBuffer(1) / 10.0f);
    delay(1000);

    HTTPClient http;
    String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?temp1=" + temp1 + "&humi1=" + humi1 + "&lux=";
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
  vTaskDelay(xDelay10000ms);
}
//==================taskThree====================//
//==================taskFour=====================//
void taskFour(void *parameter) {
  Serial.println(String("taskFour Blynk and Timer"));
  while (true) {
    Blynk.run();
    timer.run();
    vTaskDelay(xDelay1000ms);
  }
}
//==================taskFour=====================//

//===================Date Time===================//
void DateTime() {
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
  //Blynk.virtualWrite(V18, currentTime);
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
  //Blynk.virtualWrite(V19, currentDate);
  String command = "currentDate.txt=\"" + String(currentDate) + "\"";
  Serial.print(command);
  endNextionCommand();
  String command1 = "currentDate.txt=\"" + String(currentDate) + "\"";
  Serial.print(command1);
  endNextionCommand();

  String command2 = "currentTime.txt=\"" + String(currentTime) + "\"";
  Serial.print(command2);
  endNextionCommand();
  String command3 = "currentTime.txt=\"" + String(currentTime) + "\"";
  Serial.print(command3);
  endNextionCommand();
  delay(1000);
}
//===================Date Time==================//
//==============endNextionCommand===============//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//==============endNextionCommand===============//
//================Loop Function=================//
void loop() {
  nexLoop(nex_listen_list);
  //Check Wi-Fi
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
  //Blynk.run();
  //timer.run();
}
//================Loop Function=================//
//=========Check Blynk connected Status=========//
void checkBlynkStatus() {
  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    wifiFlag = 1;
    Serial.println("Blynk Not Connected");
    digitalWrite(wifiLed, LOW);
  }
  if (isconnected == true) {
    wifiFlag = 0;
    if (!fetch_blynk_state) {
      digitalWrite(wifiLed, HIGH);
    }
  }
}
//=========Check Blynk connected Status=========//
//===============Blynk Connected================//
BLYNK_CONNECTED() {
  // Request the latest state from the server
  if (fetch_blynk_state) {
    digitalWrite(wifiLed, HIGH);
    Blynk.syncAll();
  }
}
//===============Blynk Connected================//
//=======BTN Blynk ON-OFF SW3 Mbrelay1==========//
BLYNK_WRITE(V5) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x00, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x00, 0x0200);
  }
}
//=======BTN Blynk ON-OFF SW3 Mbrelay1==========//
//========BTN Blynk ON-OFF SW4 Mbrelay2=========//
BLYNK_WRITE(V6) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0200);
  }
}
//========BTN Blynk ON-OFF SW4 Mbrelay2=========//
//========BTN Blynk ON-OFF SW5 Mbrelay3=========//
BLYNK_WRITE(V7) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0200);
  }
}
//========BTN Blynk ON-OFF SW5 Mbrelay3=========//
//========BTN Blynk ON-OFF SW6 Mbrelay4=========//
BLYNK_WRITE(V8) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0200);
  }
}
//========BTN Blynk ON-OFF SW6 Mbrelay4=========//
//========BTN Blynk ON-OFF SW7 Mbrelay5=========//
BLYNK_WRITE(V9) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0200);
  }
}
//========BTN Blynk ON-OFF SW7 Mbrelay5=========//
//========BTN Blynk ON-OFF SW8 Mbrelay6=========//
BLYNK_WRITE(V10) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0200);
  }
}
//========BTN Blynk ON-OFF SW8 Mbrelay6=========//
