/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 10 Sep, 2023
    Update On: 18 Sep, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************/
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
#define INFLUXDB_URL "http://192.168.1.254:8086"
#define INFLUXDB_TOKEN "S4yK5cCTDxsCj9iyJt8He6BPukKtmvT0tc4qhszWijRwt-bXB4dSnqg3rQDfz5mDdzoi22QbmVpsvrJUy7yqMg=="
#define INFLUXDB_ORG "30af41e2ba56dce7"
#define INFLUXDB_BUCKET "esp32_dht11"
// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
// Declare Data point
Point sensor("dht11");
//===========Influxdb============//
//========Wi-Fi Credentials======//
//WiFi AP SSID
#define WIFI_SSID "smf001"
// WiFi password
#define WIFI_PASSWORD "0814111142"
//========Wi-Fi Credentials======//
//=========Time zone info========//
#define TZ_INFO "UTC7"
//=========Time zone info========//
//===========DHT11,DHT22=========//
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
#include "DHT.h"
#define DHTPIN 18     // Digital pin connected to the DHT sensor
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
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
String GAS_ID = "AKfycbxGa-u7N-gmqSui3KFBb5dS0OgmL11hvXHQCYhhVgXRqxD596DhfRUiRSo8q7zYHCGj";  //Google Script id from deploy app
//===========GGSheet=============//
//=========Setup Function========//
void setup() {
  Serial.begin(9600);
  //DHT11,DHT22 
  dht.begin();  

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
  timer.setInterval(1000L, dht2influx);
  timer.setInterval(10000, sendData2GGSheet);
}
//=========Setup Function========//
//=========DHT2Influxdb==========//
void dht2influx() 
{
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

  sensor.clearFields();                   //Clear fields for reusing the point. Tags will remain untouched
  sensor.addField("Temperature", t);      // Store measured value into point
  sensor.addField("Humidity", h);         // Store measured value into point
  

  if (wifiMulti.run() != WL_CONNECTED)  //Check WiFi connection and reconnect if needed
    Serial.println("Wifi connection lost");

  if (!client.writePoint(sensor))  //Write data point
  {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
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
//=========Loop Function=========//
void loop() {
 timer.run();
}