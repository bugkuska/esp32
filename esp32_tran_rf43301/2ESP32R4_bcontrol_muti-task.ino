/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 6 Mar, 2023
    Update On: 30 June, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    1. Multi-tasking Programing
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1. ezButton https://github.com/ArduinoGetStarted/button/archive/refs/heads/master.zip
  *  *********************************************************************************************
*/

//==================EEPROM===================//
#include <EEPROM.h>
#define EEPROM_SIZE 4
//==================EEPROM===================//
//==================zButton==================//
#include <ezButton.h>
ezButton button1(34);  //Manual button1
ezButton button2(35);  //Manual button2
ezButton button3(36);  //Manual button3
ezButton button4(39);  //Manual button4
//==================zButton==================//
//================Relay State================//
bool state_relay1 = LOW;
bool state_relay2 = LOW;
bool state_relay3 = LOW;
bool state_relay4 = LOW;
//================Relay State================//

//=========Define IO connect to relay========//
#define relay1 25  //Relay channel1
#define relay2 26  //Relay channel2
#define relay3 33  //Relay channel3
#define relay4 32  //Relay channel4
//=========Define IO connect to relay========//

// สร้างตัวแปรเก็บค่า Tick ที่แปลงจากเวลาที่เป็น millisecond
const TickType_t xDelay2000ms = pdMS_TO_TICKS(2000);
// สร้างตัวแปร TaskHandle สำหรับแต่ละ Task
TaskHandle_t Task1 = NULL;
TaskHandle_t Task2 = NULL;
TaskHandle_t Task3 = NULL;
// สร้างตัวแปรเก็บค่าเป็น integer เพื่อเอาไปใช้ใน Task
int passValue = 0;

//===============Setup function==============//
void setup() {
  Serial.begin(9600);

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

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

  digitalWrite(relay1, !state_relay1);
  digitalWrite(relay2, !state_relay2);
  digitalWrite(relay3, !state_relay3);
  digitalWrite(relay4, !state_relay4);

  state_relay1 = EEPROM.read(0);
  state_relay2 = EEPROM.read(1);
  state_relay3 = EEPROM.read(2);
  state_relay4 = EEPROM.read(3);

  digitalWrite(relay1, state_relay1);
  digitalWrite(relay2, state_relay2);
  digitalWrite(relay3, state_relay3);
  digitalWrite(relay4, state_relay4);

  Serial.printf("relay1 is %s \n", state_relay1 ? "ON" : "OFF");
  Serial.printf("relay2 is %s \n", state_relay2 ? "ON" : "OFF");
  Serial.printf("relay3 is %s \n", state_relay3 ? "ON" : "OFF");
  Serial.printf("relay4 is %s \n", state_relay4 ? "ON" : "OFF");

  delay(1000);
  // สร้าง Task1 โดยใช้ฟังก์ชัน func1_Task() พร้อมทั้งส่งผ่านค่าตัวแปร PassVale เข้าไปคำนวนใน Task
  xTaskCreatePinnedToCore(func1_Task, "Task1", 10000, (void *)passValue, 1, &Task1, 0);
  // สร้าง Task2 โดยใช้ฟังก์ชัน func2_Task()
  xTaskCreatePinnedToCore(func2_Task, "Task2", 10000, NULL, 1, &Task2, 0);
  // สร้าง Task3 โดยใช้ฟังก์ชัน func3_Task()
  xTaskCreatePinnedToCore(func3_Task, "Task3", 10000, NULL, 1, &Task3, 0);
}
//===============Setup function==============//

//===============Loop function===============//
void loop() {
  button_control();
}
//===============Loop function===============//

//==========funtion ที่ทำงานใน Task1===========//
void func1_Task(void *pvvalue) {
  int f1param = (int)pvvalue;  //แปลงค่าตัวแปรที่ส่งผ่านมาเป็น Integer
  while (1) {
    Serial.println(String("hello from Task1 : count >> ") + f1param);
    f1param++;
    vTaskDelay(xDelay2000ms);  // Delay Task นี้ 2 วินาที
  }
}
//==========funtion ที่ทำงานใน Task1===========//

//==========funtion ที่ทำงานใน Task2===========//
void func2_Task(void *pvParam) {
  while (1) {
    Serial.println(String("hello from Task2"));
    vTaskDelay(xDelay2000ms);  // Delay Task นี้ 2 วินาที
  }
}
//==========funtion ที่ทำงานใน Task2===========//

//==========funtion ที่ทำงานใน Task3===========//
void func3_Task(void *pvParam) {
  while (1) {
    Serial.println(String("hello from Task3"));
    vTaskDelay(xDelay2000ms);  // Delay Task นี้ 2 วินาที
  }
}
//==========funtion ที่ทำงานใน Task3===========//

//==========Button Control function==========//
void button_control() {
  button1.loop();
  if (button1.isPressed()) {
    control_relay(1, relay1, state_relay1);
  }
  button2.loop();
  if (button2.isPressed()) {
    control_relay(2, relay2, state_relay2);
  }
  button3.loop();
  if (button3.isPressed()) {
    control_relay(3, relay3, state_relay3);
  }
  button4.loop();
  if (button4.isPressed()) {
    control_relay(4, relay4, state_relay4);
  }
}
//==========Button Control function==========//

//==========Relay Control function===========//
void control_relay(int relay_no, int relay_pin, boolean &status) {
  status = !status;
  digitalWrite(relay_pin, status);
  EEPROM.write(relay_no - 1, status);
  EEPROM.commit();
  String text = (status) ? "ON" : "OFF";
  Serial.println("Relay" + String(relay_no) + " is " + text);
}
//==========Relay Control function===========//
