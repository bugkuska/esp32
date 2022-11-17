/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 18 Nov, 2022
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1.Download and install ITEADLIB_Arduino_Nextion library:
    https://github.com/itead/ITEADLIB_Arduino_Nextion
  *  *********************************************************************************************
    Wiring Nextion to ESP32
    5V >> 5V
    GND >> GND
    RX >> TX0
    TX >> RX2
  *Note : ระหว่าง upload source-code ให้ถอดสาย RX2 ออกจากบอร์ด ESP32 พอ upload source-code ค่อยต่อเข้าไปใหม่
    Hoo ya, have fun....
 *************************************************************************************************/

//===============DHT11================//
#include "DHT.h"
#define DHTPIN 15
#define DHTTYPE DHT11
float temperature = 0.0f;
float humidity = 0.0f;
DHT dht(DHTPIN, DHTTYPE);
//===============DHT11================//

#define sw1 sw1
#define sw2 sw2
#define sw3 sw3
#define sw4 sw4
//===============Nextion==============//
#include "Nextion.h"
// Nextion Objects, ada 5 tombol
// (page id, component id, component name)
NexButton b0 = NexButton(0, 4, "b0");
NexButton b1 = NexButton(0, 5, "b1");
NexButton b2 = NexButton(0, 6, "b2");
NexButton b3 = NexButton(0, 7, "b3");
//NexButton b4 = NexButton(0, 7, "b4");
// Declare variable global
bool statusb0 = false;
bool statusb1 = false;
bool statusb2 = false;
bool statusb3 = false;
// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
  &b0,
  &b1,
  &b2,
  &b3,
  NULL
};

//ON,OFF Relay1
void b0PushCallback(void *ptr) {
  if (statusb0 == false) {
    digitalWrite(sw1, LOW);
    statusb0 = true;
    //set button color RED
    b0.Set_background_color_bco(63488);
  } else {
    digitalWrite(sw1, HIGH);
    statusb0 = false;
    //reset button color
    b0.Set_background_color_bco(48631);
  }
}

//ON,OFF Relay2
void b1PushCallback(void *ptr) {
  if (statusb1 == false) {
    digitalWrite(sw2, LOW);
    statusb1 = true;
    //set button color RED
    b1.Set_background_color_bco(63488);
  } else {
    digitalWrite(sw2, HIGH);
    statusb1 = false;
    //reset button color
    b1.Set_background_color_bco(48631);
  }
}

//ON,OFF Relay3
void b2PushCallback(void *ptr) {
  if (statusb2 == false) {
    digitalWrite(sw3, LOW);
    statusb2 = true;
    //set button color RED
    b2.Set_background_color_bco(63488);
  } else {
    digitalWrite(sw3, HIGH);
    statusb2 = false;
    //reset button color
    b2.Set_background_color_bco(48631);
  }
}

//ON,OFF Relay4
void b3PushCallback(void *ptr) {
  if (statusb3 == false) {
    digitalWrite(sw4, LOW);
    statusb3 = true;
    //set button color RED
    b3.Set_background_color_bco(63488);
  } else {
    digitalWrite(sw4, HIGH);
    statusb3 = false;
    //reset button color
    b3.Set_background_color_bco(48631);
  }
}

//===========Setup Function===========//
void setup() {
  Serial.begin(9600);
  //Initialize Nextion Library
  nexInit();

  //DHT11
  dht.begin();

  // Register relay (output)
  pinMode(sw1, OUTPUT);
  pinMode(sw2, OUTPUT);
  pinMode(sw3, OUTPUT);
  pinMode(sw4, OUTPUT);

  // Register the push/pop event callback function
  b0.attachPush(b0PushCallback, &b0);
  b1.attachPush(b1PushCallback, &b1);
  b2.attachPush(b2PushCallback, &b2);
  b3.attachPush(b3PushCallback, &b3);

  delay(2000);
  //Matikan semua relay
  digitalWrite(sw1, HIGH);
  digitalWrite(sw2, HIGH);
  digitalWrite(sw3, HIGH);
  digitalWrite(sw4, HIGH);
  delay(500);
}
//===========Setup Function===========//

//===========Loop Function============//
void loop() {
  //When push/pop event occured execute component in touch event list
  nexLoop(nex_listen_list);

  readSensor();
  sendHumidityToNextion();
  sendTemperatureToNextion();
  delay(2000);
}
//===========Loop Function============//

//===============DHT11================//
void readSensor() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
}

void sendHumidityToNextion() {
  String command = "humidity.txt=\"" + String(humidity) + "\"";
  Serial.print(command);
  endNextionCommand();
}

void sendTemperatureToNextion() {
  String command = "temperature.txt=\"" + String(temperature, 1) + "\"";
  Serial.print(command);
  endNextionCommand();
}
//===============DHT11================//

void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
