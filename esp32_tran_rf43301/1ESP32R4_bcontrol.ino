/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 6 Mar, 2023
    Update On: 30 June, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1. ezButton https://github.com/ArduinoGetStarted/button/archive/refs/heads/master.zip
  *  *********************************************************************************************
*/
//====================EEPROM===================//
#include <EEPROM.h>
#define EEPROM_SIZE 4
//====================EEPROM===================//
//=====================zButton==================//
#include <ezButton.h>
ezButton button1(34);  //Manual button1
ezButton button2(35);  //Manual button2
ezButton button3(36);  //Manual button3
ezButton button4(39);  //Manual button4
//=====================zButton==================//
//=================Relay State==================//
bool state_relay1 = LOW;
bool state_relay2 = LOW;
bool state_relay3 = LOW;
bool state_relay4 = LOW;
//=================Relay State=================//

//=========Define IO connect to relay==========//
#define relay1 25  //Relay channel1
#define relay2 26  //Relay channel2
#define relay3 33  //Relay channel3
#define relay4 32  //Relay channel4
//=========Define IO connect to relay==========//

//===============Setup function================//
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
}
//===============Setup function================//

//===========Button Control function===========//
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
//===========Button Control function===========//

//===========Relay Control function============//
void control_relay(int relay_no, int relay_pin, boolean &status) {
  status = !status;
  digitalWrite(relay_pin, status);
  EEPROM.write(relay_no - 1, status);
  EEPROM.commit();
  String text = (status) ? "ON" : "OFF";
  Serial.println("Relay" + String(relay_no) + " is " + text);
}
//===========Relay Control function============//

//================Loop function================//
void loop() {
  button_control();
}
//================Loop function================//