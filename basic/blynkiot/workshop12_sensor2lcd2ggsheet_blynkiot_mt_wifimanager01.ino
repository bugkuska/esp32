/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 28 June, 2023
    Update On: 2 Aug, 2023
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
 *  *********************************************************************************************
    1. Multi-tasking Programming 
    2. Channel Relay
    3. 1 Digital Sensor (DHT11/DHT22)
    4. 2 Analog Sensor (SoilMoisture, MQ4)
    5. Display data & control via Blynk webdashboard & blynk mobile application
    6. Data logger to Google sheet
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1. DHT Sensor Library https://github.com/adafruit/DHT-sensor-library
    2. Blynk Library https://github.com/blynkkk/blynk-library/archive/refs/heads/master.zip
    3. Modbus Master https://github.com/4-20ma/ModbusMaster/archive/refs/heads/master.zip
    4. Simple Timer https://github.com/jfturcot/SimpleTimer/archive/refs/heads/master.zip
    5. Wi-Fi Manager https://github.com/tzapu/WiFiManager
    6. ArduinoJson Ver 5.13.4   //https://github.com/bblanchon/ArduinoJson
    7. NTPClient https://github.com/arduino-libraries/NTPClient/archive/refs/heads/master.zip
    8. I2C https://github.com/bugkuska/esp32/raw/main/basic/lcd/LiquidCrystal_i2c.zip
  *  *********************************************************************************************
 *************************************************************************************************/
/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID ""                    //Template ID from Blynk.console
#define BLYNK_TEMPLATE_NAME ""                      //Template Name from Blynk.console
#define BLYNK_AUTH_TOKEN ""  //Auth token from Blynk.console
#define BLYNK_FIRMWARE_VERSION "0.1.0"
bool fetch_blynk_state = true;  //true or false
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = BLYNK_AUTH_TOKEN;
//===============Wi-Fi Manager===============//
#include <FS.h>  //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
//Callback notifying us of the need to save config
bool shouldSaveConfig = false;
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
//===============Wi-Fi Manager===============//
//Define pin for clear and config WiFi config
#define AP_Config 14
//================SimpleTimer================//
#include <SimpleTimer.h>
SimpleTimer timer;
//================SimpleTimer================//
// สร้างตัวแปรเก็บค่า Tick ที่แปลงจากเวลาที่เป็น millisecond
const TickType_t xDelay3000ms = pdMS_TO_TICKS(3000);    //Display Sensor data (DHT11/DHT22, SoilMoisture, MQ4) on LCD2004
const TickType_t xDelay10000ms = pdMS_TO_TICKS(10000);  //Sensor data (DHT11/DHT22, SoilMoisture, MQ4) to blynk
const TickType_t xDelay60000ms = pdMS_TO_TICKS(60000);  //Sensor data (DHT11/DHT22, SoilMoisture, MQ4) to Google Sheet
// สร้างตัวแปร TaskHandle สำหรับแต่ละ Task
TaskHandle_t Task1 = NULL;  //Display Data on LCD2004
TaskHandle_t Task2 = NULL;  //DHT11,DHT22 to Blynk 2.0
TaskHandle_t Task3 = NULL;  //Soilmoisture to Blynk 2.0
TaskHandle_t Task4 = NULL;  //MQ4-Methane to Blynk 2.0
TaskHandle_t Task5 = NULL;  //Sensor data (DHT11/DHT22, SoilMoisture, MQ4) to Google Sheet
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
//char ssid[] = "";      //ชื่อ Wi-Fi รองรับคลื่น 2.4GHz เท่านั้น
//char pass[] = "";  //รหัสเชื่อมต่อ Wi-Fi
//int wifiFlag = 0;
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//==============Senddata2GGSheet===============//
#include <WiFi.h>
#include <HTTPClient.h>
const char *host = "script.google.com";
const char *httpsPort = "443";
String GAS_ID = "";  //Deployment ID from deploy app
//==============================================//
/*
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
*/
//==============initWi-Fi Manager===============//
void initWiFiManager() {
  //read configuration from FS json
  Serial.println("mounting FS...");  //แสดงข้อความใน Serial Monitor
  if (SPIFFS.begin(true)) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
        } else {
          Serial.println("failed to load json config");  //แสดงข้อความใน Serial Monitor
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");  //แสดงข้อความใน Serial Monitor
  }
  //end read
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  for (int i = 5; i > -1; i--) {  // นับเวลาถอยหลัง 5 วินาทีก่อนกดปุ่ม AP Config
    delay(500);
    Serial.print(String(i) + " ");  //แสดงข้อความใน Serial Monitor
  }


  if (digitalRead(AP_Config) == LOW) {
    Serial.println("Button Pressed");                     //แสดงข้อความใน Serial Monitor
    wifiManager.resetSettings();                          //ให้ล้างค่า SSID และ Password ที่เคยบันทึกไว้
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();  //load the flash-saved configs
    esp_wifi_init(&cfg);                                  //initiate and allocate wifi resources (does not matter if connection fails)
    delay(2000);                                          //wait a bit
    if (esp_wifi_restore() != ESP_OK) {
      Serial.println("WiFi is not initialized by esp_wifi_init ");
    } else {
      Serial.println("WiFi Configurations Cleared!");
    }
    //continue
    //delay(1000);
    //esp_restart(); //just my reset configs routine...
  }

  wifiManager.setTimeout(120);
  //ใช้ได้ 2 กรณี
  //1. เมื่อกดปุ่มเพื่อ Config ค่า AP แล้ว จะขึ้นชื่อ AP ที่เราตั้งขึ้น
  //   ช่วงนี้ให้เราทำการตั้งค่า SSID+Password หรืออื่นๆทั้งหมด ภายใน 60 วินาที ก่อน AP จะหมดเวลา
  //   ไม่เช่นนั้น เมื่อครบเวลา 60 วินาที MCU จะ Reset เริ่มต้นใหม่ ให้เราตั้งค่าอีกครั้งภายใน 60 วินาที
  //2. ช่วงไฟดับ Modem router + MCU จะดับทั้งคู่ และเมื่อมีไฟมา ทั้งคู่ก็เริ่มทำงานเช่นกัน
  //   โดยปกติ Modem router จะ Boot ช้ากว่า  MCU ทำให้ MCU กลับไปเป็น AP รอให้เราตั้งค่าใหม่
  //   ดังนั้น AP จะรอเวลาให้เราตั้งค่า 60 วินาที ถ้าไม่มีการตั้งค่าใดๆ เมื่อครบ 60 วินาที MCU จะ Reset อีกครั้ง
  //   ถ้า Modem router  Boot และใช้งานได้ภายใน 60 วินาที และหลังจากที่ MCU Resset และเริ่มทำงานใหม่
  //   ก็จะสามารถเชื่อมต่อกับ  Modem router ที่ Boot และใช้งานได้แล้ว  ได้  ระบบจะทำงานปกติ
  if (!wifiManager.autoConnect("Config-Node1", "0814111142")) {  //ชื่อ Soft AP ที่เราต้องเชื่อมต่อเพื่อเข้าหน้าตั้งค่าการเชื่อมต่อ Wi-Fi
    Serial.println("failed to connect and hit timeout");         //แสดงข้อความใน Serial Monitor
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();  //แก้ เดิม ESP.reset(); ใน Esp8266
    //delay(5000);
  }
  Serial.println("Connected.......OK!)");  //แสดงข้อความใน Serial Monitor
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");  //แสดงข้อความใน Serial Monitor
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  Serial.println("local ip");  //แสดงข้อความใน Serial Monitor
  delay(100);
  Serial.println(WiFi.localIP());  //แสดงข้อความใน Serial Monitor
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
}
//==============initWi-Fi Manager===============//

//=================Setup Function===============//
void setup() {
  Serial.begin(9600);
  dht.begin();  //เริ่มอ่านข้อมูลจาก Sensor DHT11/DHT22
  lcd.begin();      // initialize LCD2004
  lcd.backlight();  // turn on LCD backlight
  // Set IO pinMode for relay
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(ledblynk, OUTPUT);
  pinMode(AP_Config, INPUT_PULLUP);  //กำหนดโหมดใช้งานให้กับขา AP_Config เป็นขา กดปุ่ม ค้าง เพื่อตั้งค่า AP config
  //Set default relay status at boot
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  digitalWrite(ledblynk, LOW);
  /*
  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
*/
  //Wi-Fi Manager
  initWiFiManager();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  //Begin config blynk 
  Blynk.config(auth);
  delay(1000);
  //Set timer interval for read function
  timer.setInterval(30000L, checkBlynkStatus);  // เช็คการเชื่อมต่อไปที่ Blynk server ทุกๆ 30 วินาที
  //timer.setInterval(60000L, sendData2GGSheet);  //ส่งค่าไปยัง Google Sheet ทุกๆ 1 นาที

  /*********************
  - Task function
  - String with name of task
  - Stack size in words
  - Parameter passed as input of the task
  - Priority of the task
  - Task handle
  - Core  
  *********************/
  // สร้าง Task1 โดยใช้ฟังก์ชัน func1_Task() Display Data on LCD2004
  xTaskCreatePinnedToCore(func1_Task, "Task1", 10000, NULL, 1, &Task1, 1);
  // สร้าง Task2 โดยใช้ฟังก์ชัน func2_Task() DHT11,DHT22 to Blynk 2.0
  xTaskCreatePinnedToCore(func2_Task, "Task2", 10000, NULL, 2, &Task2, 1);
  // สร้าง Task3 โดยใช้ฟังก์ชัน func3_Task() Soilmoisture to Blynk 2.0
  xTaskCreatePinnedToCore(func3_Task, "Task3", 10000, NULL, 3, &Task3, 1);
  // สร้าง Task4 โดยใช้ฟังก์ชัน func4_Task() MQ4-Methane to Blynk 2.0
  xTaskCreatePinnedToCore(func4_Task, "Task4", 10000, NULL, 4, &Task4, 1);
  // สร้าง Task5 โดยใช้ฟังก์ชัน func5_Task() Sensor data (DHT11/DHT22, SoilMoisture, MQ4) to Google Sheet
  xTaskCreatePinnedToCore(func5_Task, "Task5", 10000, NULL, 5, &Task5, 1);
}
//=================Setup Function===============//
//========Check Blynk connected Status==========//
void checkBlynkStatus() {
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
/*
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
*/
//=============funtion ที่ทำงานใน Task1===========//
void func1_Task(void *pvParam) {
  while (1) {
    //Serial.println(String("Task1: Display Data on LCD2004"));
    //LCD2004 Display
    //Workshop1-DHT11/DHT22
    float h = dht.readHumidity();
    float t = dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit
    //Serial.print("Temperature:");
    //Serial.println(t);
    //Serial.print("Humidity:");
    //Serial.println(h);
    //delay(1000);

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
    //delay(1000);

    //Workshop7-I2C LCD2004
    //lcd.begin();
    //lcd.backlight();
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
    vTaskDelay(xDelay3000ms);  // Delay Task นี้ 3 วินาที
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
    int mq4;
    mq4 = analogRead(INPUT_2);
    Serial.print("Methane gas ADC : ");
    Serial.println(mq4);
    delay(1000);
    Blynk.virtualWrite(V8, mq4);
    vTaskDelay(xDelay10000ms);  // Delay Task นี้ 10 วินาที
  }
}
//============funtion ที่ทำงานใน Task4============//
//============funtion ที่ทำงานใน Task5============//
void func5_Task(void *pvParam) {
  while (1) {
    Serial.println(String("Task5: Sensor Data to Google Sheet"));
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

    vTaskDelay(xDelay60000ms);  // Delay Task นี้ 1 นาที
  }
}
//============funtion ที่ทำงานใน Task4============//
//================Loop Function=================//
void loop() {
  Blynk.run();
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
