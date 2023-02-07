/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID ""  //Blynk Template ID
#define BLYNK_DEVICE_NAME ""  //Blynk Device Name
#define BLYNK_AUTH_TOKEN ""   //Blynk auth token
#define BLYNK_FIRMWARE_VERSION "0.1.0"
bool fetch_blynk_state = true;  //true or false
//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = BLYNK_AUTH_TOKEN;
//==============Modbus-Master==============//
//V1 Relay1
//V2 temperature
//V3 Humidity
//V4 Ultrasonic

//=========Your WiFi credentials.=========//
// Set password to "" for open networks.
char ssid[] = ""; //ชื่อ Wi-Fi
char pass[] = ""; //รหัสในการเชื่อมต่อ Wi-Fi
int wifiFlag = 0;
#define wifiLed 2  //D2
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//=========Your WiFi credentials.=========//

//===========IO connect to relay==========//
#define sw1 19  //Map to Blynk virtual pin V1
//===========IO connect to relay==========//

//Timer สำหรับกำหนดค่าเวลาการอ่านค่าจากเซ็นเซอร์
#include <SimpleTimer.h>  //https://github.com/bugkuska/esp32/raw/main/basic/xProject/SimpleTimer-master.zip
SimpleTimer timer;

//dht11
#include <DHT.h>  //https://github.com/adafruit/DHT-sensor-library
#define DHTPIN 18
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//ultra sonic
// defines pins numbers
const int trigPin = 4;
const int echoPin = 5;
// defines variables
long duration;
int distance;
/*
//LCD 2002
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  //https://github.com/bugkuska/esp32/raw/main/basic/lcd/LiquidCrystal_i2c.zip
LiquidCrystal_I2C lcd(0x27, 16, 2);
*/

//================LCD 2004=================//
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  //https://github.com/bugkuska/esp32/raw/main/basic/lcd/LiquidCrystal_i2c.zip
LiquidCrystal_I2C lcd(0x27, 20, 4);
//SDA21
//SCL22
//================LCD 2004=================//

#include <Servo.h>
Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards
int pos = 0;  // variable to store the servo position

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
    Blynk.syncAll();
  }
}
//============Blynk Connected============//

//================Setup Function==============//
void setup() {
  Serial.begin(9600);

  //Servo
  myservo.attach(13);  // attaches the servo on pin 13 to the servo object

  //set io pinMode for ultra sonic
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input
  // Set IO pinMode for relay
  pinMode(sw1, OUTPUT);
  pinMode(wifiLed, OUTPUT);

  //Set default relay status at boot
  digitalWrite(sw1, LOW);
  digitalWrite(wifiLed, LOW);
  delay(500);

  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  //begin read dht
  dht.begin();

  //LCD 2004
  // initialize LCD
  lcd.begin();
  // turn on LCD backlight
  lcd.backlight();

  timer.setInterval(2000L, sensordata);  //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  Blynk.config(auth);
  delay(1000);

  if (!fetch_blynk_state) {
    Blynk.virtualWrite(V1, HIGH);
  }
}
//================Setup Function==============//

//==========Function อ่านข้อมูล DHT=============//
void sensordata() {
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
  delay(1000);

  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);

  //Ultra sonic
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  delay(1000);
  Blynk.virtualWrite(V4, distance);
}
//==========Function อ่านข้อมูล DHT=============//

//===============Function loop=================//
void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit

  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  delay(1000);

  //LCD2004
  lcd.begin();
  lcd.backlight();

  lcd.setCursor(3, 0);
  lcd.print("**==IoT RERU==**");

  lcd.setCursor(1, 1);
  lcd.print("TEMP = ");
  lcd.print(t);
  lcd.print("  C");

  lcd.setCursor(1, 2);
  lcd.print("HUMI = ");
  lcd.print(h);
  lcd.print("  %");

  lcd.setCursor(1, 3);
  lcd.print("Distance = ");
  lcd.print(distance);
  lcd.print(" cm");

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
//=============Function loop==============//

//=========BTN Blynk ON-OFF SW1==========//
BLYNK_WRITE(V1) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(sw1, HIGH);
  } else {
    digitalWrite(sw1, LOW);
  }
}
//=========BTN Blynk ON-OFF SW1==========//


BLYNK_WRITE(V5) {
  pos = 0;
  myservo.write(pos);
  //delay(2000);
}

BLYNK_WRITE(V6) {
  pos = 45;
  myservo.write(pos);
}

BLYNK_WRITE(V7) {
  pos = 90;
  myservo.write(pos);
}

BLYNK_WRITE(V8) {
  pos = 135;
  myservo.write(pos);
}

BLYNK_WRITE(V9) {
  pos = 180;
  myservo.write(pos);
}
