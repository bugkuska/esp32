/*************************************************************************************************
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
#define BLYNK_TEMPLATE_ID "TMPL6LLU6yAKW"
#define BLYNK_TEMPLATE_NAME "DSD5ESP32x"
#define BLYNK_AUTH_TOKEN "JD5wLWqZ6eTqherhZMV_ovGg2ie8rnWj"
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
// สร้างตัวแปรเก็บค่า Tick ที่แปลงจากเวลาที่เป็น millisecond
const TickType_t xDelay1000ms = pdMS_TO_TICKS(1000);    //Data to LCD2004
const TickType_t xDelay10000ms = pdMS_TO_TICKS(10000);  //Data to blynk
// สร้างตัวแปร TaskHandle สำหรับแต่ละ Task
TaskHandle_t Task1 = NULL;
TaskHandle_t Task2 = NULL;
TaskHandle_t Task3 = NULL;
TaskHandle_t Task4 = NULL;
//=========Define IO connect to relay========//
#define relay1 25    //Relay channel1
#define relay2 33    //Relay channel2
#define relay3 32    //Relay channel3
#define relay4 23    //Relay channel4
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
char ssid[] = "smf001";      //ชื่อ Wi-Fi รองรับคลื่น 2.4GHz เท่านั้น
char pass[] = "0814111142";  //รหัสเชื่อมต่อ Wi-Fi
int wifiFlag = 0;
//#define wifiLed 2  //D2
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//==============Senddata2GGSheet===============//
#include <WiFi.h>
#include <HTTPClient.h>
const char *host = "script.google.com";
const char *httpsPort = "443";
String GAS_ID = "AKfycbyxtdiZC8QQXYoDITog_z2BfK82qdwstR8R31Ov3g2Ni8Xq-MzSUtJPlnhCooiWqj2eHQ";  //Google Script id from deploy app
//==============================================//
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
  //pinMode(wifiLed, OUTPUT);

  //Set default relay status at boot
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  digitalWrite(ledblynk, LOW);
  //digitalWrite(wifiLed, LOW);

  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  timer.setInterval(10000L, checkBlynkStatus);  // check if Blynk server is connected every 10 seconds
  //timer.setInterval(5000L, dhtsensor);          //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  //timer.setInterval(5000L, soilmoisture);       //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  //timer.setInterval(5000L, mq4methane);         //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  timer.setInterval(10000L, sendData2GGSheet);  //อ่านค่าเซ็นเซอร์ทุกๆ 10 วินาที

  Blynk.config(auth);
  delay(1000);
  if (!fetch_blynk_state) {
  }

  /*Task function
  String with name of task
  Stack size in words
  Parameter passed as input of the task
  Priority of the task
  Task handle
  Core  
  */
  // สร้าง Task1 โดยใช้ฟังก์ชัน func1_Task()
  xTaskCreatePinnedToCore(func1_Task, "Task1", 10000, NULL, 1, &Task1, 1);
  // สร้าง Task2 โดยใช้ฟังก์ชัน func2_Task()
  xTaskCreatePinnedToCore(func2_Task, "Task2", 10000, NULL, 2, &Task2, 1);
  // สร้าง Task3 โดยใช้ฟังก์ชัน func3_Task()
  xTaskCreatePinnedToCore(func3_Task, "Task3", 10000, NULL, 3, &Task3, 1);
  // สร้าง Task4 โดยใช้ฟังก์ชัน func4_Task()
  xTaskCreatePinnedToCore(func4_Task, "Task4", 10000, NULL, 4, &Task4, 1);
}
//=================Setup Function===============//

//==============Wi-Fi Connection================//
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("\nConnected to the WiFi network");
  delay(2000);
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}
//==============Wi-Fi Connection================//

//========Check Blynk connected Status==========//
void checkBlynkStatus() {  // called every 10 seconds by SimpleTimer
  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    Serial.println("Blynk Not Connected");
    digitalWrite(ledblynk, LOW);
  }
  if (isconnected == true) {
    if (!fetch_blynk_state) {
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
/*
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
//=============Soilmoisturd Sensor==============//
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
//=============Soilmoisturd Sensor==============//

//=============MQ4-Methane Sensor===============//
void mq4methane() {
  int sensor_analog2;
  sensor_analog2 = analogRead(INPUT_2);
  Serial.print("Methane gas ADC : ");
  Serial.println(sensor_analog2);
  delay(1000);
  Blynk.virtualWrite(V8, sensor_analog2);
}
//=============MQ4-Methane Sensor===============//
*/
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
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?t=" + t + "&h=" + h + "&moisture_percentage1=" + moisture_percentage1 + "&sensor_analog2=" + sensor_analog2;
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

//=============funtion ที่ทำงานใน Task1===========//
void func1_Task(void *pvParam) {
  while (1) {
    Serial.println(String("Task1: Display Data on LCD2004"));
    //LCD2004 Display
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
    vTaskDelay(xDelay1000ms);  // Delay Task นี้ 1 วินาที
  }
}
//=============funtion ที่ทำงานใน Task1===========//

//=============funtion ที่ทำงานใน Task2===========//
void func2_Task(void *pvParam) {
  while (1) {
    Serial.println(String("Task2: DHT11/DHT22 to Blynk 2.0"));

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
    vTaskDelay(xDelay10000ms);  // Delay Task นี้ 10 วินาที
  }
}
//=============funtion ที่ทำงานใน Task2===========//

//=============funtion ที่ทำงานใน Task3===========//
void func3_Task(void *pvParam) {
  while (1) {
    Serial.println(String("Task3: Soilmoisture to Blynk 2.0"));
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
    vTaskDelay(xDelay10000ms);  // Delay Task นี้ 10 วินาที
  }
}
//=============funtion ที่ทำงานใน Task3===========//

//============funtion ที่ทำงานใน Task4============//
void func4_Task(void *pvParam) {
  while (1) {
    Serial.println(String("Task4: MQ4-Methane to Blynk 2.0"));
    int sensor_analog2;
    sensor_analog2 = analogRead(INPUT_2);
    Serial.print("Methane gas ADC : ");
    Serial.println(sensor_analog2);
    delay(1000);
    Blynk.virtualWrite(V8, sensor_analog2);
    vTaskDelay(xDelay10000ms);  // Delay Task นี้ 10 วินาที
  }
}
//============funtion ที่ทำงานใน Task4============//

//================Loop Function=================//
void loop() {
  /*
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
  lcd.print("  C ");

  lcd.setCursor(1, 1);
  lcd.print("HUMI =  ");
  lcd.print(h);
  lcd.print("  % ");

  lcd.setCursor(1, 2);
  lcd.print("Soil =  ");
  lcd.print(moisture_percentage1);
  lcd.print("  % ");

  lcd.setCursor(1, 3);
  lcd.print("MQ4 =  ");
  lcd.print(sensor_analog2);
  lcd.print("  ppm ");
*/
  Blynk.run();
  timer.run();

  //Check Wi-Fi
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
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