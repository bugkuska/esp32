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
//============Modbus Master=============//
#include <ModbusMaster.h>
#define RX1 16  //RX
#define TX1 17  //TX
//============Modbus Master=============//
//=============Modbus Object============//
ModbusMaster node1;  //Slave ID1 Modbus Temperature & Humidity XY-MD02

//=============Modbus Object============//
//=============Simple Timer=============//
#include <SimpleTimer.h>
SimpleTimer timer;
//=============Simple Timer=============//
//================GPIO==================//
#define Relay1_sw1 25
#define Widget_Btn_sw1 V1
#define Relay2_sw2 26
#define Widget_Btn_sw2 V2
#define Relay3_sw3 33
#define Widget_Btn_sw3 V3
#define Relay4_sw4 32
#define Widget_Btn_sw4 V4
//================GPIO==================//
//V1 SW1
//V2 SW2
//V3 SW3
//V4 SW4
//V5 XY-MD02 Temerature
//V6 XY-MD02 Humidity
//================GPIO==================//
//=========Your WiFi credentials.========//
// Set password to "" for open networks.
char ssid[] = "";  //แก้ชื่อกับรหัส Wi-Fi
char pass[] = "";
int wifiFlag = 0;
#define wifiLed 2  //D2
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//=========Your WiFi credentials.========//

//============Wi-Fi Connection===========//
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
  }
  Serial.println(WiFi.localIP());
}
//============Wi-Fi Connection===========//
//============Setup Function=============//
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX1, TX1);  //RX1=16,TX1=17
  node1.begin(1, Serial2);                    //SoilMoisture Slave ID1
  delay(100);

  // Setup Pin Mode
  pinMode(Relay1_sw1, OUTPUT);  // ESP32 PIN GPIO25
  pinMode(Relay2_sw2, OUTPUT);  // ESP32 PIN GPIO26
  pinMode(Relay3_sw3, OUTPUT);  // ESP32 PIN GPIO33
  pinMode(Relay4_sw4, OUTPUT);  // ESP32 PIN GPIO32
  pinMode(wifiLed, OUTPUT);  // ESP32 PIN GPIO2

  // Set Defult Relay Status
  digitalWrite(Relay1_sw1, LOW);  // ESP32 PIN GPIO25
  digitalWrite(Relay2_sw2, LOW);  // ESP32 PIN GPIO26
  digitalWrite(Relay3_sw3, LOW);  // ESP32 PIN GPIO33
  digitalWrite(Relay4_sw4, LOW);  // ESP32 PIN GPIO32
  digitalWrite(wifiLed, LOW);  // ESP32 PIN GPIO19
  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  timer.setInterval(10000L, checkBlynkStatus);  // check if Blynk server is connected every 10 seconds
  timer.setInterval(5000L, xymd02);
  Blynk.config(auth);
  delay(1000);

  if (!fetch_blynk_state) {
  }
}
//============Setup Function=============//
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
//========XY-MD02 Temp&Humi==============//
void xymd02()//Function ในการอ่านค่าจาก Sensor 
{
  uint8_t result1; 
  float temp1 = (node1.getResponseBuffer(0)/10.0f);
  float humi1 = (node1.getResponseBuffer(1)/10.0f);

  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2); //Function 04, Read 2 registers starting at 2)
  if (result1 == node1.ku8MBSuccess)
  {
    Serial.print("Temp: ");
    Serial.println(node1.getResponseBuffer(0)/10.0f);
    Serial.print("Humi: ");
    Serial.println(node1.getResponseBuffer(1)/10.0f);
  }
  delay(1000);
  Blynk.virtualWrite(V5, temp1);
  Blynk.virtualWrite(V6, humi1);
}
//========XY-MD02 Temp&Humi==============//
//=============Loop Function=============//
void loop() {
  //When push/pop event occured execute component in touch event list

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

//==============ON-OFF SW1==============//
BLYNK_WRITE(Widget_Btn_sw1) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(Relay1_sw1, HIGH);
  } else {
    digitalWrite(Relay1_sw1, LOW);
  }
}
//==============ON-OFF SW1==============//

//==============ON-OFF SW2==============//
BLYNK_WRITE(Widget_Btn_sw2) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(Relay2_sw2, HIGH);
  } else {
    digitalWrite(Relay2_sw2, LOW);
  }
}
//==============ON-OFF SW2==============//

//==============ON-OFF SW3==============//
BLYNK_WRITE(Widget_Btn_sw3) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(Relay3_sw3, HIGH);
  } else {
    digitalWrite(Relay3_sw3, LOW);
  }
}
//==============ON-OFF SW3==============//

//==============ON-OFF SW4==============//
BLYNK_WRITE(Widget_Btn_sw4) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(Relay4_sw4, HIGH);
  } else {
    digitalWrite(Relay4_sw4, LOW);
  }
}
//==============ON-OFF SW4==============//
