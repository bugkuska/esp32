//Connect to Wi-Fi
#include <WiFi.h>
char ssid[] = "";      //ชื่อ SSID ที่เราต้องการเชื่อมต่อ
char pass[] = "";  //รหัสผ่าน WI-FI

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

//ldr
#define INPUT_1 36  //Analog input1 for read ldr data

//soilmoisture
#define INPUT_2 39  //Analog input3 for read soil moisture data

//LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  //https://github.com/bugkuska/esp32/raw/main/basic/lcd/LiquidCrystal_i2c.zip
LiquidCrystal_I2C lcd(0x27, 16, 2);

//================Setup Function==============//
void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("WiFi Connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  //begin read dht
  dht.begin();

  //set io pinMode for ultra sonic
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input

  timer.setInterval(2000L, sensordata);  //อ่านค่าเซ็นเซอร์ทุกๆ 5 วินาที
  //timer.setInterval(2000L, ultrasonic); //อ่านค่าเซ็นเซอร์ทุกๆ 2 วินาที
  //timer.setInterval(2000L, ldr);        //อ่านค่าเซ็นเซอร์ทุกๆ 2 วินาที
  //timer.setInterval(2000L, soil);       //อ่านค่าเซ็นเซอร์ทุกๆ 2 วินาที
}
//================Setup Function==============//

//==========Function อ่านข้อมูล DHT=============//
void sensordata() {
  // Wait a few seconds between measurements.
  //delay(2000);

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
  delay(1000);

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

  //LDR
  float ldr_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1);
  Serial.print("Law LDR data 1:");
  Serial.println(sensor_analog1);
  ldr_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));

  Serial.print("LDR Percentage 1 = ");
  Serial.print(ldr_percentage1);
  Serial.print("%\n\n");
  delay(1000);
  
  //SoilMoisture
   float moisture_percentage3;
  int sensor_analog3;
  sensor_analog3 = analogRead(INPUT_2);
  Serial.print("Law Soil data 3:");
  Serial.println(sensor_analog3);
  moisture_percentage3 = (100 - ((sensor_analog3 / 4095.00) * 100));

  Serial.print("Moisture Percentage 3= ");
  Serial.print(moisture_percentage3);
  Serial.print("%\n\n");
  delay(1000);

}
//==========Function อ่านข้อมูล DHT=============//

/*
//=======Function อ่านข้อมูล Ultra sonic=========//
void ultrasonic() {
  // Clears the trigPin
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
  //delay(2000);
}
//=======Function อ่านข้อมูล Ultra sonic=========//
*/
/*
//===========Function อ่านข้อมูล LDR=============//
void ldr() {
  float ldr_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1);
  Serial.print("Law LDR data 1:");
  Serial.println(sensor_analog1);
  ldr_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));

  Serial.print("LDR Percentage 1 = ");
  Serial.print(ldr_percentage1);
  Serial.print("%\n\n");
  delay(1000);
}
//===========Function อ่านข้อมูล LDR=============//
*/
/*
//========Function อ่านข้อมูล SoilMoisture=======//
void soil() {
  float moisture_percentage3;
  int sensor_analog3;
  sensor_analog3 = analogRead(INPUT_2);
  Serial.print("Law Soil data 3:");
  Serial.println(sensor_analog3);
  moisture_percentage3 = (100 - ((sensor_analog3 / 4095.00) * 100));

  Serial.print("Moisture Percentage 3= ");
  Serial.print(moisture_percentage3);
  Serial.print("%\n\n");
  delay(1000);
}
//========Function อ่านข้อมูล SoilMoisture=======//
*/
//===============Function loop=================//
void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit

  /*
  Serial.print("Temperature:");
  Serial.println(t);
  Serial.print("Humidity:");
  Serial.println(h);
  delay(1000);
*/
  //LCD
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("TEMP=");
  lcd.print(t);
  lcd.print(" C");

  lcd.setCursor(1, 1);
  lcd.print("HUMI=");
  lcd.print(h);
  lcd.print(" %");

  timer.run();
}
//===============Function loop=================//
