/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 28 June, 2023
    Update On: 28 July, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
Map Blynk VirtualPin 
V1 Relay Channel 1
V2 Relay Channel 2
V3 Relay Channel 3
V4 Relay Channel 4
V5 DHT11/DHT22 Temperature
V6 DHT11/DHT22 Humidity
V7 SoilMoisture Sensor
V8 MQ4-Methane Gas Sensor
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
//================SimpleTimer================//
#include <SimpleTimer.h>
SimpleTimer timer;
//================SimpleTimer================//
//=========Define IO connect to relay========//
#define relay1 25   //Relay channel1
#define relay2 33   //Relay channel2
#define relay3 32   //Relay channel3
#define relay4 23   //Relay channel4
#define ledblynk 2  //Led for check blynk connection
//=========Define IO connect to relay========//
//==========Workshop1-DHT11/DHT22============//
//dht11,dht22
#include <DHT.h>  //https://github.com/adafruit/DHT-sensor-library
#define DHTPIN 18
//#define DHTTYPE DHT11
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
//======Workshop4-SoilMoisture sensor=======//
#define INPUT_1 35
//=======Workshpp5-MQ4 Methane Gas==========//
#define INPUT_2 34
//==========Workshop7-I2C LCD2004===========//
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  //https://github.com/bugkuska/esp32/raw/main/basic/lcd/LiquidCrystal_i2c.zip
LiquidCrystal_I2C lcd(0x27, 20, 4);
//========Workshop8-Connect to Wi-Fi===========//
char ssid[] = "";      //ชื่อ Wi-Fi รองรับคลื่น 2.4GHz เท่านั้น
char pass[] = "";  //รหัสเชื่อมต่อ Wi-Fi
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//==============Senddata2GGSheet===============//
#include <WiFi.h>
#include <HTTPClient.h>
const char *host = "script.google.com";
const char *httpsPort = "443";
String GAS_ID = "";  //Google Script id from deploy app
//==============================================//

//==============Wi-Fi Connection================//
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(2000);
  }
  Serial.println("\nConnected to the WiFi network");
  delay(2000);
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}
//==============Wi-Fi Connection================//

//=================Setup Function===============//
void setup() {
  Serial.begin(9600);
  dht.begin();
  //LCD 2004
  lcd.begin();      // initialize LCD
  lcd.backlight();  // turn on LCD backlight

  // Set IO pinMode for relay
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(ledblynk, OUTPUT);

  //Set default relay status at boot
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  digitalWrite(ledblynk, LOW);

  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  Blynk.config(auth);
  //Blynk.begin(BLYNK_AUTH_TOKEN, "ssid", "pass");

  timer.setInterval(10000L, checkBlynkStatus);  // check if Blynk server is connected every 10 seconds
  //timer.setInterval(5000L, dhtsensor);          //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  //timer.setInterval(5000L, soilmoisture);       //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  //timer.setInterval(5000L, mq4methane);         //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(10000L, sendData2GGSheet);  //อ่านค่าเซ็นเซอร์ทุกๆ 10 วินาที

  //delay(1000);
  /*if (!fetch_blynk_state) {
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
    digitalWrite(relay3, LOW);
    digitalWrite(relay4, LOW);
    Blynk.syncAll();
  }*/
}
//=================Setup Function===============//

//========Check Blynk connected Status==========//
void checkBlynkStatus() {  // called every 10 seconds by SimpleTimer
  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    Serial.println("Blynk Not Connected");
    digitalWrite(ledblynk, LOW);
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
    digitalWrite(relay3, LOW);
    digitalWrite(relay4, LOW);
  }
  if (isconnected == true) {
    if (fetch_blynk_state) {
      digitalWrite(ledblynk, HIGH);
    }
  }
}
//========Check Blynk connected Status==========//

//===============Blynk Connected================//
BLYNK_CONNECTED() {
  // Request the latest state from the server
  if (fetch_blynk_state) {
    digitalWrite(ledblynk, HIGH);
    Blynk.syncAll();
  }
}
//===============Blynk Connected================//

//=============DHT11/DHT22 Sensor===============//
void dhtsensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();      // Read temperature as Celsius (the default)
  float f = dht.readTemperature(true);  // Read temperature as Fahrenheit (isFahrenheit = true)
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  float hif = dht.computeHeatIndex(f, h);         // Compute heat index in Fahrenheit (the default)
  float hic = dht.computeHeatIndex(t, h, false);  // Compute heat index in Celsius (isFahreheit = false)
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  Blynk.virtualWrite(V5, t);
  Blynk.virtualWrite(V6, h);
}
//=============DHT11/DHT22 Sensor===============//

//============Soilmoisture Sensor===============//
void soilmoisture() {
  float moisture_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1);
  Serial.print("Law Soil data 1:");
  Serial.println(sensor_analog1);
  moisture_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));

  Serial.print("Moisture Percentage 1= ");
  Serial.print(moisture_percentage1);
  Serial.print("%\n\n");

  Blynk.virtualWrite(V7, moisture_percentage1);
}
//============Soilmoisture Sensor===============//

//=============MQ4-Methane Sensor===============//
void mq4methane() {
  int mq4;
  mq4 = analogRead(INPUT_2);
  Serial.print("Methane gas ADC : ");
  Serial.println(mq4);
  delay(1000);
  Blynk.virtualWrite(V8, mq4);
}
//=============MQ4-Methane Sensor===============//

//=============SendData2GGSheet=================//
void sendData2GGSheet() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit

  float moisture_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1);
  moisture_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));

  int mq4;
  mq4 = analogRead(INPUT_2);

  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?t=" + t + "&h=" + h + "&moisture_percentage1=" + moisture_percentage1 + "&mq4=" + mq4;
  //Serial.print(url);
  Serial.println("Posting Temperature and humidity data to Google Sheet");
  //---------------------------------------------------------------------
  //starts posting data to google sheet
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);
  //getting response from google sheet
  String payload;
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Payload: " + payload);
  }
  http.end();
}
//=============SendData2GGSheet=================//

//================Loop Function=================//
void loop() {
  Blynk.run();
  timer.run();

  //Workshop1-DHT11/DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit
  /*Serial.print("Temperature:");
  Serial.println(t);
  Serial.print("Humidity:");
  Serial.println(h);
  delay(1000);*/

  //Workshop4-SoilMoisture Sensor
  float moisture_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1);
  //Serial.print("Law Soil data 1:");
  //Serial.println(sensor_analog1);
  moisture_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));
  //Serial.print("Moisture Percentage 1= ");
  //Serial.print(moisture_percentage1);
  //Serial.print("%\n\n");
  //delay(1000);

  //Workshop5-MQ4 Methane Gas
  int mq4;
  mq4 = analogRead(INPUT_2);
  //Serial.print("Methane gas ADC : ");
  //Serial.println(mq4);
  // delay(1000);


  //Workshop7-I2C LCD2004
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("TEMP =  ");
  lcd.print(t);
  lcd.setCursor(16, 0);
  lcd.print("C");

  lcd.setCursor(1, 1);
  lcd.print("HUMI =  ");
  lcd.print(h);
  lcd.setCursor(16, 1);
  lcd.print("%");

  lcd.setCursor(1, 2);
  lcd.print("Soil =  ");
  lcd.print(moisture_percentage1);
  lcd.setCursor(16, 2);
  lcd.print("%");

  lcd.setCursor(1, 3);
  lcd.print("MQ4  =  ");
  lcd.print(mq4);
  lcd.setCursor(16, 3);
  lcd.print("PPM");

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
//================Loop Function=================//

//==========BTN Blynk ON-OFF Relay1=============//
BLYNK_WRITE(V1) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(relay1, HIGH);
  } else {
    digitalWrite(relay1, LOW);
  }
}
//==========BTN Blynk ON-OFF Relay1============//

//==========BTN Blynk ON-OFF Relay2============//
BLYNK_WRITE(V2) {
  int valuebtn2 = param.asInt();
  if (valuebtn2 == 1) {
    digitalWrite(relay2, HIGH);
  } else {
    digitalWrite(relay2, LOW);
  }
}
//==========BTN Blynk ON-OFF Relay2============//

//==========BTN Blynk ON-OFF Relay3============//
BLYNK_WRITE(V3) {
  int valuebtn3 = param.asInt();
  if (valuebtn3 == 1) {
    digitalWrite(relay3, HIGH);
  } else {
    digitalWrite(relay3, LOW);
  }
}
//==========BTN Blynk ON-OFF Relay3============//

//==========BTN Blynk ON-OFF Relay4============//
BLYNK_WRITE(V4) {
  int valuebtn4 = param.asInt();
  if (valuebtn4 == 1) {
    digitalWrite(relay4, HIGH);
  } else {
    digitalWrite(relay4, LOW);
  }
}
//==========BTN Blynk ON-OFF Relay4===========//
