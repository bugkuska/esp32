/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 28 June, 2023
    Update On: 28 July, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************/
//Workshop1-DHT11/DHT22
//dht11,dht22
#include <DHT.h>  //https://github.com/adafruit/DHT-sensor-library
#define DHTPIN 18
//#define DHTTYPE DHT11
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
//Workshop4-SoilMoisture sensor
#define INPUT_1 35
//Workshpp5-MQ4 Methane Gas
#define INPUT_2 34
//Workshop7-I2C LCD2004
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  //https://github.com/bugkuska/esp32/raw/main/basic/lcd/LiquidCrystal_i2c.zip
LiquidCrystal_I2C lcd(0x27, 20, 4);
//Workshop8-Connect to Wi-Fi
//===========Your WiFi credentials.============//
char ssid[] = "";      //ชื่อ Wi-Fi รองรับคลื่น 2.4GHz เท่านั้น
char pass[] = "";  //รหัสเชื่อมต่อ Wi-Fi
int wifiFlag = 0;
#define wifiLed 2  //D2
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//===========Your WiFi credentials.============//
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

//==============Wi-Fi Connection================//
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    //delay(1000);
  }
  Serial.println(WiFi.localIP());
  digitalWrite(wifiLed, HIGH);
}
//==============Wi-Fi Connection================//

//=================Setup Function===============//
void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, LOW);

  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  timer.setInterval(5000L, sendData2GGSheet);
}
//=================Setup Function===============//

//=============SendData2GGSheet=================//
void sendData2GGSheet() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit

  float moisture_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1);
  moisture_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));

  int sensor_analog2;
  sensor_analog2 = analogRead(INPUT_2);

  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?t=" + t + "&h=" + h + "&moisture_percentage1="+moisture_percentage1 + "&sensor_analog2=" + sensor_analog2;
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
//=============SendData2GGSheet=================//

void loop() {
  //Workshop1-DHT11/DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit
  Serial.print("Temperature:");
  Serial.println(t);
  Serial.print("Humidity:");
  Serial.println(h);
  delay(1000);

  //Workshop4-SoilMoisture Sensor
  float moisture_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1);
  Serial.print("Law Soil data 1:");
  Serial.println(sensor_analog1);
  moisture_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));
  Serial.print("Moisture Percentage 1= ");
  Serial.print(moisture_percentage1);
  Serial.print("%\n\n");
  delay(1000);

  //Workshop5-MQ4 Methane Gas
  int sensor_analog2;
  sensor_analog2 = analogRead(INPUT_2);
  Serial.print("Methane gas ADC : ");
  Serial.println(sensor_analog2);
  delay(1000);
  
    //Workshop7-I2C LCD2004
    lcd.begin();
    lcd.backlight();
    lcd.setCursor(1, 0);
    lcd.print("TEMP =  ");
    lcd.print(t);
    lcd.setCursor(16,0);
    lcd.print("C");

    lcd.setCursor(1, 1);
    lcd.print("HUMI =  ");
    lcd.print(h);
    lcd.setCursor(16,1);
    lcd.print("%");

    lcd.setCursor(1, 2);
    lcd.print("Soil =  ");
    lcd.print(moisture_percentage1);
    lcd.setCursor(16,2);
    lcd.print("%");

    lcd.setCursor(1, 3);
    lcd.print("MQ4  =  ");
    lcd.print(sensor_analog2);
    lcd.setCursor(16,3);
    lcd.print("PPM");

  timer.run();

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
