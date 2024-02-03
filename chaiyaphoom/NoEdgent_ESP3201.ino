/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID "TMPL6wKQ7zBA4"
#define BLYNK_TEMPLATE_NAME "Smartfarm"
#define BLYNK_AUTH_TOKEN "t8H7XCWqvNZnhWAkSdshGYI-EPtddw8n"
#define BLYNK_FIRMWARE_VERSION "0.2.0"
bool fetch_blynk_state = true;  //true or false
//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = BLYNK_AUTH_TOKEN;
//============Modbus Master=============//
#include <ModbusMaster.h>
#define RX1 26  //RO,RX
#define TX1 27  //DI,TX
//============Modbus Master=============//
//=============Modbus Object============//
ModbusMaster node1;  //Slave ID1 SoilMoisture
ModbusMaster node2;  //Slave ID2 3IN1 Sensor
ModbusMaster node3;  //Slave ID3 STHEC_NPK Sensor
//=============Modbus Object============//

//=============Simple Timer=============//
#include <SimpleTimer.h>
SimpleTimer timer;
//=============Simple Timer=============//
//================GPIO==================//
#define Relay1_sw1 25
#define Widget_Btn_sw1 V1
#define ledblynk 19
//================GPIO==================//
//V1 SW1-Valve1
//V5 SoilMoisture
//V6 Nitrogen
//V7 Phoporous
//V8 3Photasium
//================LCD 2004==============//
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
//SDA21
//SCL22
//================LCD 2004==============//

//=========Your WiFi credentials.========//
// Set password to "" for open networks.
char ssid[] = "smf001";               //แก้ชื่อกับรหัส Wi-Fi
char pass[] = "0814111142";
int wifiFlag = 0;
#define wifiLed 2  //D2
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//=========Your WiFi credentials.========//

//============Senddata2GGSheet===========//
#include <WiFi.h>
#include <HTTPClient.h>
const char *host = "script.google.com";
const char *httpsPort = "443";
String GAS_ID = "AKfycbxGHyFO9E0BQG9S5Og4aarUn7tZlkTPM5alsonovH_jrBFGYjMdwFKVu0mwRvjg0koGvw";  //Google Script id from deploy app
//============Senddata2GGSheet===========//

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
  node2.begin(2, Serial2);                    //3IN1 Slave ID2
  node3.begin(3, Serial2);                    //STHEC_NPK Slave ID3
  delay(100);

  //LCD 2004
  // initialize LCD
  lcd.begin();
  // turn on LCD backlight
  lcd.backlight();

  // Setup Pin Mode
  pinMode(Relay1_sw1, OUTPUT);  // ESP32 PIN GPIO25
  pinMode(ledblynk, OUTPUT);    // ESP32 PIN GPIO19
  pinMode(wifiLed, OUTPUT);     // ESP32 PIN GPIO2

  // Set Defult Relay Status
  digitalWrite(Relay1_sw1, LOW);  // ESP32 PIN GPIO25
  digitalWrite(ledblynk, HIGH);   // ESP32 PIN GPIO19

  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  timer.setInterval(10000L, checkBlynkStatus);  // check if Blynk server is connected every 10 seconds
  timer.setInterval(5000L, SoilMoisture01);
  timer.setInterval(5000L, NPK);
  timer.setInterval(10000L, GzwsSensorData);
  timer.setInterval(30000L, sendData2GGSheet);

  Blynk.config(auth);
  delay(1000);

  if (!fetch_blynk_state) {
    //Blynk.virtualWrite(V1, HIGH);
    //Blynk.virtualWrite(V2, HIGH);
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
    digitalWrite(ledblynk, LOW);
    Blynk.syncAll();
  }
}
//============Blynk Connected============//

//=======SoilMoisture Sensor============//
void SoilMoisture01() {
  uint8_t result;
  float soil_01 = (node1.getResponseBuffer(2) / 10.0f);

  Serial.println("Get Soil Moisture Data");
  result = node1.readHoldingRegisters(0x0000, 3);  // Read 2 registers starting at 1)
  if (result == node1.ku8MBSuccess) {
    Serial.print("Soil Moisture Sensor: ");
    Serial.println(node1.getResponseBuffer(2) / 10.0f);
  }
  delay(100);
  Blynk.virtualWrite(V5, soil_01);
}
//=======SoilMoisture Sensor============//

//======GZWS Humi&Temp and Light=======//
void GzwsSensorData() {
  //======node2 Slave ID 2======//
  uint8_t result1;
  float humi1 = (node2.getResponseBuffer(0) / 10.0f);
  float temp1 = (node2.getResponseBuffer(1) / 10.0f);
  float light1 = (node2.getResponseBuffer(2));
  float light_per1;
  Serial.println("GZWS Data1");
  result1 = node2.readHoldingRegisters(0x0000, 3);  // Read 3 registers starting at 1)
  if (result1 == node2.ku8MBSuccess) {
    Serial.print("Humi1: ");
    Serial.println(node2.getResponseBuffer(0) / 10.0f);
    Serial.print("Temp1: ");
    Serial.println(node2.getResponseBuffer(1) / 10.0f);
    light_per1 = (light1 = node2.getResponseBuffer(2));
    light_per1 = map(light_per1, 0, 65535, 0, 100);
    Serial.print("Light1: ");
    Serial.println(light_per1);
  }
  delay(1000);
  //Blynk.virtualWrite(V6, humi1);
  //Blynk.virtualWrite(V7, temp1);
  //Blynk.virtualWrite(V8, light_per1);
}
//======GZWS Humi&Temp and Light=======//

//========STHEC_NPK Sensor Data========//
void NPK() {
  //======node3 Slave ID 3======//
  uint8_t result1;
  float soil_6in1 = (node3.getResponseBuffer(0) / 10.0f);
  float soiltemp_6in1 = (node3.getResponseBuffer(1) / 10.0f);
  float soilec_6in1 = (node3.getResponseBuffer(2));
  float soilph_6in1 = (node3.getResponseBuffer(3) / 10.0f);
  float soilnit_6in1 = (node3.getResponseBuffer(5));
  float soilpho_6in1 = (node3.getResponseBuffer(6));
  float soilpot_6in1 = (node3.getResponseBuffer(7));

  Serial.println("6IN1 Sensor Data");
  result1 = node3.readHoldingRegisters(0x0000, 8);  // Read 8 registers starting at 1)
  if (result1 == node3.ku8MBSuccess) {
    Serial.print("SoilHumidity: ");
    Serial.println(node3.getResponseBuffer(0) / 10.0f);
    Serial.print("SoilTemperature: ");
    Serial.println(node3.getResponseBuffer(1) / 10.0f);

    Serial.print("SoilEC: ");
    Serial.println(node3.getResponseBuffer(2));

    Serial.print("SoilPh: ");
    Serial.println(node3.getResponseBuffer(3) / 10.0f);

    Serial.print("SoilNitrogen: ");
    Serial.println(node3.getResponseBuffer(5));

    Serial.print("SoilPhosporous: ");
    Serial.println(node3.getResponseBuffer(6));

    Serial.print("SoilPhotasium: ");
    Serial.println(node3.getResponseBuffer(7));
  }
  delay(1000);
  Blynk.virtualWrite(V6, soilnit_6in1);
  Blynk.virtualWrite(V7, soilpho_6in1);
  Blynk.virtualWrite(V8, soilpot_6in1);
}
//========STHEC_NPK Sensor Data========//

//========SendData2GGSheet=============//
void sendData2GGSheet() {
  float soil_01 = (node1.getResponseBuffer(2) / 10.0f);

  float humi1 = (node2.getResponseBuffer(0) / 10.0f);
  float temp1 = (node2.getResponseBuffer(1) / 10.0f);
  float light1 = (node2.getResponseBuffer(2));


  float soil_6in1 = (node3.getResponseBuffer(0) / 10.0f);
  float soiltemp_6in1 = (node3.getResponseBuffer(1) / 10.0f);
  float soilec_6in1 = (node3.getResponseBuffer(2));
  float soilph_6in1 = (node3.getResponseBuffer(3) / 10.0f);
  float soilnit_6in1 = (node3.getResponseBuffer(5));
  float soilpho_6in1 = (node3.getResponseBuffer(6));
  float soilpot_6in1 = (node3.getResponseBuffer(7));

  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?soil1=" + soil_01 + "&nit=" + soilnit_6in1 + "&pho=" + soilpho_6in1 + "&pot=" + soilpot_6in1 + "&temp=" + temp1 + "&humi=" + humi1;
  //Serial.print(url);
  Serial.println("Posting Sensor Data to Google Sheet");
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
//========SendData2GGSheet=============//

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

  //LCD 2004
  float soil_01 = (node1.getResponseBuffer(2) / 10.0f);
  lcd.setCursor(0, 0);
  lcd.print("*=DR.SAMART SINTON=*");

  lcd.setCursor(0, 1);
  lcd.print("Tel:080-799-9907");

  lcd.setCursor(0, 2);
  lcd.print("Soil:   ");
  lcd.print(soil_01);
  lcd.setCursor(15, 2);
  lcd.print("%");
  Blynk.run();
  timer.run();
}
//=============Loop Function=============//
