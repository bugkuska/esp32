/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 10 Sep, 2023
    Update On: 18 Sep, 2023
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
 *************************************************************************************************/
//==========Blynk 2.0============//
#define BLYNK_TEMPLATE_ID "TMPL65BMieuNc"
#define BLYNK_DEVICE_NAME "esp32"
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial
#define BLYNK_DEBUG
#define APP_DEBUG
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"
bool fetch_blynk_state = true;  //true or false
//==========Blynk 2.0============//
//==========Multi-Wi-Fi==========//
#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif
//==========Multi-Wi-Fi==========//
//===========Influxdb============//
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#define INFLUXDB_URL ""
#define INFLUXDB_TOKEN ""
#define INFLUXDB_ORG ""
#define INFLUXDB_BUCKET ""
// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
// Declare Data point
Point sensor("dht11");
//===========Influxdb============//
//========Wi-Fi Credentials======//
//WiFi AP SSID
#define WIFI_SSID ""
// WiFi password
#define WIFI_PASSWORD ""
//========Wi-Fi Credentials======//
//=========Time zone info========//
#define TZ_INFO "UTC7"
//=========Time zone info========//
//===========DHT11,DHT22=========//
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
#include "DHT.h"
#define DHTPIN 18  // Digital pin connected to the DHT sensor
// Uncomment whatever type you're using!
#define DHTTYPE DHT11  // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
//===========DHT11,DHT22=========//
//==========SimpleTimer==========//
#include <SimpleTimer.h>
SimpleTimer timer;
//==========SimpleTimer==========//
//===========GGSheet=============//
#include <WiFi.h>
#include <HTTPClient.h>
const char *host = "script.google.com";
const char *httpsPort = "443";
String GAS_ID = "";  //Google Script id from deploy app
//===========GGSheet=============//
//==========Relay4CH=============//
#define relay1 25
#define relay2 33
#define relay3 32
#define relay4 23
//==========Relay4CH=============//
//========Setup Function=========//
void setup() {
  Serial.begin(9600);
  //DHT11,DHT22
  dht.begin();

  // Set IO pinMode for relay
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  //Set default relay status at boot
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Accurate time is necessary for certificate validation and writing in batches
  // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  delay(100);
  BlynkEdgent.begin();

  timer.setInterval(1000L, dht2influx);
  timer.setInterval(30000, sendData2GGSheet);
}
//========Setup Function=========//
//=======Blynk Connected=========//
BLYNK_CONNECTED() {
  // Request the latest state from the server
  if (fetch_blynk_state) {
    //digitalWrite(ledblynk, HIGH);
    Blynk.syncAll();
  }
}
//=======Blynk Connected=========//
//=========Loop Function=========//
void loop() {
  BlynkEdgent.run();
  timer.run();
}
//=========Loop Function=========//
//=========DHT2Influxdb==========//
void dht2influx() {
  // Wait a few seconds between measurements.
  delay(2000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

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

  sensor.clearFields();               //Clear fields for reusing the point. Tags will remain untouched
  sensor.addField("Temperature", t);  // Store measured value into point
  sensor.addField("Humidity", h);     // Store measured value into point


  if (wifiMulti.run() != WL_CONNECTED)  //Check WiFi connection and reconnect if needed
    Serial.println("Wifi connection lost");

  if (!client.writePoint(sensor))  //Write data point
  {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  delay(1000);
  Blynk.virtualWrite(V5, t);
  Blynk.virtualWrite(V6, h);
}
//=========DHT2Influxdb==========//
//==========DHT2GGSheet==========//
void sendData2GGSheet() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit

  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?t=" + t + "&h=" + h;
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
//==========DHT2GGSheet==========//
//===BTN Blynk ON-OFF Relay1=====//
BLYNK_WRITE(V1) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(relay1, HIGH);
  } else {
    digitalWrite(relay1, LOW);
  }
}
//===BTN Blynk ON-OFF Relay1=====//
//===BTN Blynk ON-OFF Relay2=====//
BLYNK_WRITE(V2) {
  int valuebtn2 = param.asInt();
  if (valuebtn2 == 1) {
    digitalWrite(relay2, HIGH);
  } else {
    digitalWrite(relay2, LOW);
  }
}
//===BTN Blynk ON-OFF Relay2=====//
//===BTN Blynk ON-OFF Relay3=====//
BLYNK_WRITE(V3) {
  int valuebtn3 = param.asInt();
  if (valuebtn3 == 1) {
    digitalWrite(relay3, HIGH);
  } else {
    digitalWrite(relay3, LOW);
  }
}
//===BTN Blynk ON-OFF Relay3=====//
//===BTN Blynk ON-OFF Relay4=====//
BLYNK_WRITE(V4) {
  int valuebtn4 = param.asInt();
  if (valuebtn4 == 1) {
    digitalWrite(relay4, HIGH);
  } else {
    digitalWrite(relay4, LOW);
  }
}
//===BTN Blynk ON-OFF Relay4=====//
