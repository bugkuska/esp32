/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 21 Mar, 2023
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
  *Note : ระหว่าง upload source-code ให้ถอดสาย RX0 ออกจากบอร์ด ESP32 พอ upload source-code ค่อยต่อเข้าไปใหม่
  2. Download and install Modbus-Master, Simple-Timer
    Hoo ya, have fun....
 *************************************************************************************************/
/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""
#define BLYNK_FIRMWARE_VERSION "0.1.0"
bool fetch_blynk_state = true;  //true or false
//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = BLYNK_AUTH_TOKEN;
//==============Modbus-Master==============//
#include <ModbusMaster.h>
#define RXD2 16
#define TXD2 17
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 XY-MD02
float temp1 = 0.0f;  //Map to Blynk virtual pin V3
float humi1 = 0.0f;  //Map to Blynk virtual pin V4
//==============Modbus-Master==============//
//Virtual Pin
//V1 GPIO18 == SW1
//V2 GPIO19 == SW2
//V3 Temperature
//V4 Humidity
//V5 LDR
//==============SimpleTimer===============//
#include <SimpleTimer.h>
SimpleTimer timer;
//==============SimpleTimer===============//

//=========Your WiFi credentials.=========//
// Set password to "" for open networks.
char ssid[] = "";  //ชื่อ Wi-Fi (คลื่น 2.4x Ghz เท่านั้น)
char pass[] = "";  // รหัสเชื่อมต่อ Wi-Fi
int wifiFlag = 0;
#define wifiLed 2  //D2
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//=========Your WiFi credentials.=========//

//===========IO connect to relay==========//
#define sw1 18  //Map to Blynk virtual pin V1
#define sw2 19  //Map to Blynk virtual pin V2
//===========IO connect to relay==========//

//==================LDR===================//
#define INPUT_ldr 34  //Map to Blynk virtual pin V5
//==================LDR===================//

//======Nextion Library and Object=======//
#include "Nextion.h"  //Nextion libraries
// Nextion Objects, ada 5 tombol
// (page id, component id, component name)
//Page 3 Dual State button
NexButton bt0 = NexButton(3, 2, "bt0");
NexButton bt1 = NexButton(3, 3, "bt1");
//Page4 Text XY-MD02
NexText textemp1 = NexText(4, 2, "textemp1");
NexText texhumi = NexText(4, 3, "texhumi");
//Page 5 Progress BarXY-MD02
NexText textemp2 = NexText(5, 2, "textemp2");
NexText texhumi2 = NexText(5, 3, "texhumi2");
NexProgressBar pgtemp1 = NexProgressBar(5, 4, "pgtemp1");
NexProgressBar pghumi1 = NexProgressBar(5, 5, "pghumi1");
//Page 6 Guage XY-MD02
NexGauge guagetemp1 = NexGauge(6, 2, "guagetemp1");
NexGauge guagehumi1 = NexGauge(6, 3, "guagehumi1");
//Page 7 Guage LDR
NexGauge guageldr = NexGauge(7, 2, "guageldr");
NexText textldr = NexText(7, 3, "textldr");

// Declare variable global
bool statusbt0 = false;
bool statusbt1 = false;

// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
  &bt0,
  &bt1,
  NULL
};
//======Nextion Library and Object=======//

//=============ON,OFF Relay1=============//
void bt0PushCallback(void *ptr) {
  if (statusbt0 == false) {
    digitalWrite(sw1, LOW);
    statusbt0 = true;
    Blynk.virtualWrite(V1, 1);
  } else if (statusbt0 == true) {
    digitalWrite(sw1, HIGH);
    statusbt0 = false;
    Blynk.virtualWrite(V1, 0);
  }
}
//=============ON,OFF Relay1=============//

//============ON,OFF Relay2==============//
void bt1PushCallback(void *ptr) {
  if (statusbt1 == false) {
    digitalWrite(sw2, LOW);
    statusbt1 = true;
    Blynk.virtualWrite(V2, 1);
  } else {
    digitalWrite(sw2, HIGH);
    statusbt1 = false;
    Blynk.virtualWrite(V2, 0);
  }
}
//============ON,OFF Relay2==============//
/*
//============Wi-Fi Connection===========//
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
//============Wi-Fi Connection===========//
*/
//============Wi-Fi Connection===========//
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    //delay(1000);
  }
  //while (WiFi.status() != WL_CONNECTED) {
  //  Serial.print('.');
  //  delay(1000);
  // }
  Serial.println(WiFi.localIP());
}
//============Wi-Fi Connection===========//

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

//============Setup Function=============//
void setup() {
  Serial.begin(9600);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node1.begin(1, Serial2);
  //Initialize Nextion Library
  nexInit();

  // Register the push/pop event callback function
  bt0.attachPush(bt0PushCallback, &bt0);
  bt1.attachPush(bt1PushCallback, &bt1);
  delay(2000);

  // Set IO pinMode for relay
  pinMode(sw1, OUTPUT);
  pinMode(sw2, OUTPUT);
  pinMode(wifiLed, OUTPUT);

  //Set default relay status at boot
  digitalWrite(sw1, HIGH);
  digitalWrite(sw2, HIGH);
  digitalWrite(wifiLed, LOW);
  delay(500);

  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  timer.setInterval(10000L, checkBlynkStatus);  // check if Blynk server is connected every 10 seconds
  timer.setInterval(10000L, xymdtoBlynk);          //อ่านค่าเซ็นเซอร์ทุกๆ 10 วินาที
  timer.setInterval(5000L, xymdtoNex);        //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(5000L, ldrtoNex);

  Blynk.config(auth);
  delay(1000);

  if (!fetch_blynk_state) {
    Blynk.virtualWrite(V1, HIGH);
    Blynk.virtualWrite(V2, HIGH);
  }
}
//============Setup Function=============//

//==========XY-MD02 to Nextion===========//
void xymdtoNex() {
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0)/10.0f);
  float humi1 = (node1.getResponseBuffer(1)/10.0f);

  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2); //Function 04, Read 2 registers starting at 2)
  if (result1 == node1.ku8MBSuccess) {
    Serial.print("Temp1: ");
    Serial.println(node1.getResponseBuffer(0)/10.0f);
    Serial.print("Humi1: ");
    Serial.println(node1.getResponseBuffer(1)/10.0f);
  }

  //Page 3 Text XY-MD02
  String command = "textemp1.txt=\"" + String(temp1) + "\"";
  Serial.print(command);
  endNextionCommand();

  String command1 = "textemp1.txt=\"" + String(temp1) + "\"";
  Serial.print(command1);
  endNextionCommand();

  String command2 = "texhumi.txt=\"" + String(humi1) + "\"";
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
//==========XY-MD02 to Nextion===========//

//=============LDRtoNextion==============//
void ldrtoNex() {
  float ldr_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_ldr);
  ldr_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));

  Serial.print("LDR Percentage 1 = ");
  Serial.print(ldr_percentage1);
  Serial.print("%\n\n");

  //Page 6 Guage LDR
  String command3 = "textldr.txt=\"" + String(ldr_percentage1) + "\"";
  Serial.print(command3);
  endNextionCommand();

  String command4 = "textldr.txt=\"" + String(ldr_percentage1) + "\"";
  Serial.print(command4);
  endNextionCommand();

  int val = map(ldr_percentage1, 0, 100, 0, 222);
  Serial.print("guageldr.val=");  //Send the object tag
  Serial.print(val);              //Send the value
  endNextionCommand();
}
//=============LDRtoNextion==============//

//===========endNextionCommand===========//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//===========endNextionCommand===========//

//===========XY-MD02 to Blynk 2.0========//
void xymdtoBlynk() {
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0)/10.0f);
  float humi1 = (node1.getResponseBuffer(1)/10.0f);

  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2); //Function 04, Read 2 registers starting at 2)
  if (result1 == node1.ku8MBSuccess) {
    Serial.print("Temp1: ");
    Serial.println(node1.getResponseBuffer(0)/10.0f);
    Serial.print("Humi1: ");
    Serial.println(node1.getResponseBuffer(1)/10.0f);
  }
  
  delay(1000);
  Blynk.virtualWrite(V3, temp1);
  Blynk.virtualWrite(V4, humi1);
}
//===========XY-MD02 to Blynk 2.0========//

//=============Loop Function=============//
void loop() {
  //When push/pop event occured execute component in touch event list
  nexLoop(nex_listen_list);

  //Check Wi-Fi connection
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }

  Blynk.run();
  timer.run();
}
//=============Loop Function=============//

//=========BTN Blynk ON-OFF SW1==========//
BLYNK_WRITE(V1) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(sw1, LOW);
  } else {
    digitalWrite(sw1, HIGH);
  }
}
//=========BTN Blynk ON-OFF SW1==========//

//=========BTN Blynk ON-OFF SW2==========//
BLYNK_WRITE(V2) {
  int valuebtn2 = param.asInt();
  if (valuebtn2 == 1) {
    digitalWrite(sw2, LOW);
  } else {
    digitalWrite(sw2, HIGH);
  }
}
//=========BTN Blynk ON-OFF SW2==========//
