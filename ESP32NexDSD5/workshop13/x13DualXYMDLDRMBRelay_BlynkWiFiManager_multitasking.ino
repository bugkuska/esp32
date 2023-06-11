/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 6 Mar, 2023
    Update On: 11 June, 2023 Multi-tasking
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1.Download and install ITEADLIB_Arduino_Nextion library:
    https://github.com/itead/ITEADLIB_Arduino_Nextion
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
#define BLYNK_TEMPLATE_ID ""  //TEMPLATE_ID จาก Blynk.console>device info
#define BLYNK_TEMPLATE_NAME "" //TEMPLATE_NAME จาก Blynk.console>device info
//#define BLYNK_AUTH_TOKEN ให้คัดลอกเอา BLYNK_AUTH_TOKEN จาก Blynk.console>device info เก็บไว้ใส่พร้อมกับการตั้งค่า Wi-Fi
#define BLYNK_FIRMWARE_VERSION "0.1.0"
bool fetch_blynk_state = true;  //true or false
//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
//================Wi-Fi Manager================//
#include <FS.h>      //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>  
//needed for library
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>  //Ver 5.13.4   //https://github.com/bblanchon/ArduinoJson
//Callback notifying us of the need to save config
bool shouldSaveConfig = false;
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
char blynk_token[34] = "";
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

//=======Nextion Library and Object=========//
#include "Nextion.h"  //Nextion libraries
// (page id, component id, component name)

//Page id 3 DualState Button
NexButton bt0 = NexButton(3, 2, "bt0");
NexButton bt1 = NexButton(3, 3, "bt1");

//Page id 4 Text XY-MD02
NexText textemp1 = NexText(4, 2, "textemp1");
NexText texhumi1 = NexText(4, 3, "texhumi1");

//Page id 5 Progress BarXY-MD02
NexText textemp2 = NexText(5, 2, "textemp2");
NexText texhumi2 = NexText(5, 3, "texhumi2");
NexProgressBar pgtemp1 = NexProgressBar(5, 4, "pgtemp1");
NexProgressBar pghumi1 = NexProgressBar(5, 5, "pghumi1");

//Page id 6 Guage XY-MD02
NexGauge guagetemp1 = NexGauge(6, 2, "guagetemp1");
NexGauge guagehumi1 = NexGauge(6, 3, "guagehumi1");

//Page id 7 Modbus Relay
NexButton bt2 = NexButton(7, 2, "bt2");
NexButton bt3 = NexButton(7, 3, "bt3");
NexButton bt4 = NexButton(7, 4, "bt4");
NexButton bt5 = NexButton(7, 5, "bt5");
NexButton bt6 = NexButton(7, 6, "bt6");
NexButton bt7 = NexButton(7, 7, "bt7");
NexButton bt8 = NexButton(7, 8, "bt8");
NexButton bt9 = NexButton(7, 9, "bt9");
//=======Nextion Library and Object=========//

// Declare variable global
bool statusbt0 = false;
bool statusbt1 = false;
//Modbus Relay
bool statusbt2 = false;
bool statusbt3 = false;
bool statusbt4 = false;
bool statusbt5 = false;
bool statusbt6 = false;
bool statusbt7 = false;
bool statusbt8 = false;
bool statusbt9 = false;
// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
  //2CH Relay
  &bt0,
  &bt1,
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
//=========Nextion Library and Object==========//

//==========Define IO connect to relay==========//
#define sw1 18
#define sw2 19

//=================Modbus-Master================//
#include <ModbusMaster.h>
#define RXD2 16         //Serial2
#define TXD2 17         //Serial2
#define RXD1 26         //Serial1
#define TXD1 27         //Serial1
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 XY-MD02
float humi1 = 0.0f;
float temp1 = 0.0f;
// instantiate ModbusMaster object
ModbusMaster node2;  //Slave ID2 Modbus RTU Relay
int8_t pool_size1;   //Pool size for Modbus Write command

//=================Modbus-Master================//

//================Task Delay====================//
const TickType_t xDelay2000ms = pdMS_TO_TICKS(2000);
const TickType_t xDelay5000ms = pdMS_TO_TICKS(5000);
const TickType_t xDelay10000ms = pdMS_TO_TICKS(10000);
//================Task Delay====================//

//================ON,OFF Relay1=================//
void bt0PushCallback(void *ptr) {
  if (statusbt0 == false) {
    digitalWrite(sw1, LOW);
    statusbt0 = true;
  } else if (statusbt0 == true) {
    digitalWrite(sw1, HIGH);
    statusbt0 = false;
  }
}
//================ON,OFF Relay1=================//

//=================ON,OFF Relay2================//
void bt1PushCallback(void *ptr) {
  if (statusbt1 == false) {
    digitalWrite(sw2, LOW);
    statusbt1 = true;
  } else {
    digitalWrite(sw2, HIGH);
    statusbt1 = false;
  }
}
//=================ON,OFF Relay2================//

//================Modbus Relay==================//
//==========ON,OFF MRelay1==========//
void bt2PushCallback(void *ptr) {
  if (statusbt2 == false) {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0100);
    //relayControl_modbusRTU(2, 1, 1);
    //digitalWrite(sw1, LOW);
    statusbt2 = true;
  } else if (statusbt2 == true) {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0200);
    //relayControl_modbusRTU(2, 1, 0);
    //digitalWrite(sw1, HIGH);
    statusbt2 = false;
  }
}
//==========ON,OFF MRelay1==========//

//==========ON,OFF MRelay2==========//
void bt3PushCallback(void *ptr) {
  if (statusbt3 == false) {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0100);
    //relayControl_modbusRTU(2, 2, 1);
    //digitalWrite(sw1, LOW);
    statusbt3 = true;
  } else if (statusbt3 == true) {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0200);
    //relayControl_modbusRTU(2, 2, 0);
    //digitalWrite(sw1, HIGH);
    statusbt3 = false;
  }
}
//==========ON,OFF MRelay2==========//

//==========ON,OFF MRelay3==========//
void bt4PushCallback(void *ptr) {
  if (statusbt4 == false) {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0100);
    //relayControl_modbusRTU(2, 3, 1);
    //digitalWrite(sw1, LOW);
    statusbt4 = true;
  } else if (statusbt4 == true) {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0200);
    //relayControl_modbusRTU(2, 3, 0);
    //digitalWrite(sw1, HIGH);
    statusbt4 = false;
  }
}
//==========ON,OFF MRelay3==========//

//==========ON,OFF MRelay4==========//
void bt5PushCallback(void *ptr) {
  if (statusbt5 == false) {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0100);
    //relayControl_modbusRTU(2, 4, 1);
    //digitalWrite(sw1, LOW);
    statusbt5 = true;
  } else if (statusbt5 == true) {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0200);
    //relayControl_modbusRTU(2, 4, 0);
    //digitalWrite(sw1, HIGH);
    statusbt5 = false;
  }
}
//==========ON,OFF MRelay4==========//

//==========ON,OFF MRelay5==========//
void bt6PushCallback(void *ptr) {
  if (statusbt6 == false) {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0100);
    //relayControl_modbusRTU(2, 5, 1);
    //digitalWrite(sw1, LOW);
    statusbt6 = true;
  } else if (statusbt6 == true) {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0200);
    //relayControl_modbusRTU(2, 5, 0);
    //digitalWrite(sw1, HIGH);
    statusbt6 = false;
  }
}
//==========ON,OFF MRelay5==========//

//==========ON,OFF MRelay6==========//
void bt7PushCallback(void *ptr) {
  if (statusbt7 == false) {
    pool_size1 = node2.writeSingleRegister(0x06, 0x0100);
    //relayControl_modbusRTU(2, 6, 1);
    //digitalWrite(sw1, LOW);
    statusbt7 = true;
  } else if (statusbt7 == true) {
    pool_size1 = node2.writeSingleRegister(0x06, 0x0200);
    //relayControl_modbusRTU(2, 6, 0);
    //digitalWrite(sw1, HIGH);
    statusbt7 = false;
  }
}
//==========ON,OFF MRelay6==========//

//==========ON,OFF MRelay7==========//
void bt8PushCallback(void *ptr) {
  if (statusbt8 == false) {
    pool_size1 = node2.writeSingleRegister(0x07, 0x0100);
    //relayControl_modbusRTU(2, 7, 1);
    //digitalWrite(sw1, LOW);
    statusbt8 = true;
  } else if (statusbt8 == true) {
    pool_size1 = node2.writeSingleRegister(0x07, 0x0200);
    //relayControl_modbusRTU(2, 7, 0);
    //digitalWrite(sw1, HIGH);
    statusbt8 = false;
  }
}
//==========ON,OFF MRelay7==========//

//==========ON,OFF MRelay8==========//
void bt9PushCallback(void *ptr) {
  if (statusbt9 == false) {
    pool_size1 = node2.writeSingleRegister(0x08, 0x0100);
    //relayControl_modbusRTU(2, 8, 1);
    //digitalWrite(sw1, LOW);
    statusbt9 = true;
  } else if (statusbt9 == true) {
    pool_size1 = node2.writeSingleRegister(0x08, 0x0200);
    //relayControl_modbusRTU(2, 8, 0);
    //digitalWrite(sw1, HIGH);
    statusbt9 = false;
  }
}
//==========ON,OFF MRelay8==========//
//================Modbus Relay==================//
//==============initWi-Fi Manager===============//
void initWiFiManager() {
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
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
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          //strcpy(mqtt_server, json["mqtt_server"]);
          //strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
  
  //WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  //WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
    WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

  //wifiManager.resetSettings();
  //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here
  //wifiManager.addParameter(&custom_mqtt_server);
  //wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_blynk_token);
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
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
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

if (!wifiManager.autoConnect("BBWiFi-Config", "0814111142")) {   //ชื่อ Soft AP ที่เราต้องเชื่อมต่อเพื่อเข้าหน้าตั้งค่าการเชื่อมต่อ Wi-Fi
    Serial.println("failed to connect and hit timeout");  //แสดงข้อความใน Serial Monitor
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
  } 
  if ((WiFi.status()!=WL_CONNECTED) )
  {
      Serial.println("failed to connect");
  } 
  else
  {
    //if you get here you have connected to the WiFi
  Serial.println("local ip");  //แสดงข้อความใน Serial Monitor
  delay(100);
  Serial.println(WiFi.localIP());  //แสดงข้อความใน Serial Monitor
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  }
  
  //read updated parameters
  //strcpy(mqtt_server, custom_mqtt_server.getValue());
  //strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(blynk_token, custom_blynk_token.getValue());
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    //json["mqtt_server"] = mqtt_server;
    //json["mqtt_port"] = mqtt_port;
    json["blynk_token"] = blynk_token;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
}
//==============initWi-Fi Manager===============//

void setup() {
  Serial.begin(9600);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); //GPIO16, GPIO17
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1); //GPIO26, GPIO27
  node1.begin(1, Serial2);          //Serial2 สำหรับอ่านข้อมูล Sensor
  node2.begin(2, Serial1);          //Serial1 สำหรับควบคุม Modbus Relay

  //Initialize Nextion Library
  nexInit();

  // Register the push/pop event callback function
  //2CH Relay
  bt0.attachPush(bt0PushCallback, &bt0);
  bt1.attachPush(bt1PushCallback, &bt1);

  //Modbus Relay
  bt2.attachPush(bt2PushCallback, &bt2);
  bt3.attachPush(bt3PushCallback, &bt3);
  bt4.attachPush(bt4PushCallback, &bt4);
  bt5.attachPush(bt5PushCallback, &bt5);
  bt6.attachPush(bt6PushCallback, &bt6);
  bt7.attachPush(bt7PushCallback, &bt7);
  bt8.attachPush(bt8PushCallback, &bt8);
  bt9.attachPush(bt9PushCallback, &bt9);

  // Set IO pinMode for relay
  pinMode(sw1, OUTPUT);
  pinMode(sw2, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  digitalWrite(sw1, HIGH);
  digitalWrite(sw2, HIGH);
  digitalWrite(wifiLed, LOW);
  delay(2000);

  initWiFiManager();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  timer.setInterval(30000L, checkBlynkStatus);  //Check สถานะการเชื่อมต่อ Blynk Server ทุกๆ 30 วินาที

  Blynk.config(blynk_token);
  delay(1000);

  if (!fetch_blynk_state) {
    Blynk.virtualWrite(V3, HIGH);
    Blynk.virtualWrite(V4, HIGH);
  }
//สร้าง Task สำหรับจัดการงานหลายงาน ตัวอย่างสร้าง 4 Task
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
//taskThree จัดการการเชื่อมต่อไปที่ Blynk
  xTaskCreate(
    taskThree,   /* Task function. */
    "TaskThree", /* String with name of task. */
    10000,       /* Stack size in words. */
    NULL,        /* Parameter passed as input of the task */
    1,           /* Priority of the task. */
    NULL);
//taskFour สำหรับอ่านข้อมูล Sensor ตัวที่ 1 และส่งค่าไปเก็บไว้บน Google Sheet
  xTaskCreate(
    taskFour,    /* Task function. */
    "TaskThree", /* String with name of task. */
    10000,       /* Stack size in words. */
    NULL,        /* Parameter passed as input of the task */
    1,           /* Priority of the task. */
    NULL);
}
//============Setup Function=============//

//===============taskOne=================//
void taskOne(void *parameter) {
  Serial.println(String("taskOne Sensor data to Nextion"));
  while (true) {
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
    delay(2000);

    //Page 3 Text XY-MD02
    String command = "textemp1.txt=\"" + String(temp1) + "\"";
    Serial.print(command);
    endNextionCommand();

    String command1 = "textemp1.txt=\"" + String(temp1) + "\"";
    Serial.print(command1);
    endNextionCommand();

    String command2 = "texhumi1.txt=\"" + String(humi1) + "\"";
    Serial.print(command2);
    endNextionCommand();

    //Page 4 Progress Bar XY-MD02
    String command3 = "textemp2.txt=\"" + String(temp1) + "\"";
    Serial.print(command3);
    endNextionCommand();

    String command4 = "textemp2.txt=\"" + String(temp1) + "\"";
    Serial.print(command4);
    endNextionCommand();

    String command5 = "texhumi2.txt=\"" + String(humi1) + "\"";
    Serial.print(command5);
    endNextionCommand();

    pgtemp1.setValue(temp1);
    Serial.print("pgtemp1.val=");
    Serial.print(temp1);
    endNextionCommand();

    pghumi1.setValue(humi1);
    Serial.print("pghumi1.val=");
    Serial.print(humi1);
    endNextionCommand();

    //Page 5 Guage XY-MD02
    int val1 = map(temp1, 0, 100, 0, 223);
    Serial.print("guagetemp1.val=");  //Send the object tag
    Serial.print(val1);               //Send the value
    endNextionCommand();

    int val2 = map(humi1, 0, 100, 0, 223);
    Serial.print("guagehumi1.val=");  //Send the object tag
    Serial.print(val2);               //Send the value
    endNextionCommand();
  }
  vTaskDelay(xDelay2000ms);
}
//===============taskOne=================//

//===============taskTwo=================//
void taskTwo(void *parameter) {
  Serial.println(String("taskTwo Sensor data to Blynk.IoT"));
  while (true) {
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
    Blynk.virtualWrite(V1, temp1);
    Blynk.virtualWrite(V2, humi1);
  }
  vTaskDelay(xDelay5000ms);
}
//===============taskTwo=================//

//==============taskThree================//
void taskThree(void *parameter) {
  Serial.println(String("taskThree Blynk Handle"));
  while (true) {
    Blynk.run();
    vTaskDelay(xDelay2000ms);
  }
}
//==============taskThree================//

//==============taskFour=================//

void taskFour(void *parameter) {
  Serial.println(String("taskFour Sensor data to Google Sheet"));
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
//==============taskFour=================//

//============endNextionCommand=============//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//============endNextionCommand=============//
//=============Loop Function=============//
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
}
//=============Loop Function=============//


//=====Check Blynk connected Status======//
void checkBlynkStatus() {  // called every 10 seconds by SimpleTimer
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
//=====Check Blynk connected Status======//

//============Blynk Connected============//
BLYNK_CONNECTED() {
  // Request the latest state from the server
  if (fetch_blynk_state) {
    digitalWrite(wifiLed, HIGH);
    Blynk.syncAll();
  }
}
//============Blynk Connected============//

//=======BTN Blynk ON-OFF SW1 Digtal Relay1========//
BLYNK_WRITE(V3) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(sw1, LOW);
  } else {
    digitalWrite(sw1, HIGH);
  }
}
//=======BTN Blynk ON-OFF SW1 Digtal Relay1========//

//=======BTN Blynk ON-OFF SW2 Digtal Relay2========//
BLYNK_WRITE(V4) {
  int valuebtn2 = param.asInt();
  if (valuebtn2 == 1) {
    digitalWrite(sw2, LOW);
  } else {
    digitalWrite(sw2, HIGH);
  }
}
//=======BTN Blynk ON-OFF SW2 Digtal Relay2========//

//=========BTN Blynk ON-OFF SW3 Mbrelay1==========//
BLYNK_WRITE(V5) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0200);
  }
}
//=========BTN Blynk ON-OFF SW3 Mbrelay1==========//

//=========BTN Blynk ON-OFF SW4 Mbrelay2==========//
BLYNK_WRITE(V6) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0200);
  }
}
//=========BTN Blynk ON-OFF SW4 Mbrelay2==========//

//=========BTN Blynk ON-OFF SW5 Mbrelay3==========//
BLYNK_WRITE(V7) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0200);
  }
}
//=========BTN Blynk ON-OFF SW5 Mbrelay3==========//

//=========BTN Blynk ON-OFF SW6 Mbrelay4==========//
BLYNK_WRITE(V8) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0200);
  }
}
//=========BTN Blynk ON-OFF SW6 Mbrelay4==========//

//=========BTN Blynk ON-OFF SW7 Mbrelay5==========//
BLYNK_WRITE(V9) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0200);
  }
}
//=========BTN Blynk ON-OFF SW7 Mbrelay5==========//

//=========BTN Blynk ON-OFF SW8 Mbrelay6==========//
BLYNK_WRITE(V10) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x06, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x06, 0x0200);
  }
}
//=========BTN Blynk ON-OFF SW8 Mbrelay6==========//
