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
#define RX1 16  //RO,RX
#define TX1 17  //DI,TX
//============Modbus Master=============//
//=============Modbus Object============//
ModbusMaster node1;  //Slave ID1 Windspeed Sensor
//=============Modbus Object============//

//=============Simple Timer=============//
#include <SimpleTimer.h>
SimpleTimer timer;
//=============Simple Timer=============//
//==============VirtualPIN==============//
//V1 Windspeed
//==============VirtualPIN==============//

//=========Your WiFi credentials.========//
// Set password to "" for open networks.
char ssid[] = "";               //แก้ชื่อกับรหัส Wi-Fi
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
  node1.begin(1, Serial2);                    //Slave ID1 Windspeed Sensor
  delay(100);

  // Setup Pin Mode
  pinMode(wifiLed, OUTPUT);     // ESP32 PIN GPIO2

  // Set Defult Relay Status
  digitalWrite(wifiLed, LOW);  // ESP32 PIN GPIO2

  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  timer.setInterval(10000L, checkBlynkStatus);  // check if Blynk server is connected every 10 seconds
  timer.setInterval(5000L, wind_rs_n01);

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

//=========Windspeed Sensor====--========//
void wind_rs_n01()
{
  uint8_t result;
  float wind01 = (node1.getResponseBuffer(0)); //Change m/s to km/h= m/s *3.6, Change km/h to m/s = km/h /3.6 
 
  Serial.println("Get Wind Speed Data");
   result = node1.readHoldingRegisters(0x0000, 1); // Read 2 registers starting at 1)
  if (result == node1.ku8MBSuccess)
  {
    Serial.print("Winspeed: ");
    Serial.println(node1.getResponseBuffer(0));
  }
  delay(100);
  Blynk.virtualWrite(V1, wind01);
}
//=======SoilMoisture Sensor============//

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
