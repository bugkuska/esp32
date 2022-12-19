/**********************************************************************************
 *  TITLE: Blynk + Manual Switch (latched) + IR + DHT11 control 4 Relays using ESP32 (Real time feedback + no WiFi control)
 *  Click on the following links to learn more. 
 *  YouTube Video: https://youtu.be/VNeT5QgH-IM
 *  Related Blog : https://iotcircuithub.com/esp32-projects/
 *  by Tech StudyCell
 *  Preferences--> Aditional boards Manager URLs : 
 *  https://dl.espressif.com/dl/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *  
 *  Download Board ESP32 : https://github.com/espressif/arduino-esp32
 *
 *  Download the libraries 
 *  Blynk Library (1.0.1):  https://github.com/blynkkk/blynk-library
 *  IRremote Library (3.6.1): https://github.com/Arduino-IRremote/Arduino-IRremote
 *  DHT Library (1.4.3): https://github.com/adafruit/DHT-sensor-library
 **********************************************************************************/

 /*************************************************************************************************
    Update By: Sompoch Tongnamtiang
    Created On: 15 Oct, 2022
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *************************************************************************************************/

/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_AUTH_TOKEN ""
#define BLYNK_FIRMWARE_VERSION "0.1.0"
//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
bool fetch_blynk_state = true;  //true or false
char auth[] = BLYNK_AUTH_TOKEN;
BlynkTimer timer;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "";
char pass[] = "";
#define wifiLed 2
int wifiFlag = 0;
//Update the HEX code of IR Remote buttons 0x<HEX CODE>
#define IR_Button_1 0xFFA25D
#define IR_Button_2 0xFF629D
#define IR_Button_3 0xFFE21D
#define IR_Button_4 0xFF22DD
#define IR_All_Off 0xFF38C7

//DHT
#include <DHT.h>
#define DHTPIN 5
// Uncomment whatever type you're using!
#define DHTTYPE DHT11  // DHT 11
//#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301
DHT dht(DHTPIN, DHTTYPE);
float temperature1 = 0;
float humidity1 = 0;

//IR Remote
#include <IRremote.h>
#define IR_RECV_PIN 15
IRrecv irrecv(IR_RECV_PIN);
decode_results results;

//RF433 Mhz
#include <RCSwitch.h>
const int rfreceive_pin = 4;
RCSwitch mySwitch = RCSwitch();

// define the GPIO connected with Relays and switches
#define RelayPin1 26
#define RelayPin2 25
#define RelayPin3 33
#define RelayPin4 32

#define SwitchPin1 36
#define SwitchPin2 39
#define SwitchPin3 34
#define SwitchPin4 35

//Change the virtual pins according the rooms
#define VPIN_BUTTON_1 V1
#define VPIN_BUTTON_2 V2
#define VPIN_BUTTON_3 V3
#define VPIN_BUTTON_4 V4
#define VPIN_BUTTON_OK V5
#define VPIN_TEMPERATURE V6
#define VPIN_HUMIDITY V7

// Relay State
bool toggleState_1 = HIGH;  //Define integer to remember the toggle state for relay 1
bool toggleState_2 = HIGH;  //Define integer to remember the toggle state for relay 2
bool toggleState_3 = HIGH;  //Define integer to remember the toggle state for relay 3
bool toggleState_4 = HIGH;  //Define integer to remember the toggle state for relay 4

// Switch State
bool SwitchState_1 = HIGH;
bool SwitchState_2 = HIGH;
bool SwitchState_3 = HIGH;
bool SwitchState_4 = HIGH;

// Setup Function
void setup() {
  Serial.begin(9600);

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);
  pinMode(RelayPin4, OUTPUT);
  pinMode(wifiLed, OUTPUT);

  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);
  pinMode(SwitchPin3, INPUT_PULLUP);
  pinMode(SwitchPin4, INPUT_PULLUP);

  //During Starting all Relays should TURN OFF
  digitalWrite(RelayPin1, !toggleState_1);
  digitalWrite(RelayPin2, !toggleState_2);
  digitalWrite(RelayPin3, !toggleState_3);
  digitalWrite(RelayPin4, !toggleState_4);
  digitalWrite(wifiLed, LOW);
  
  mySwitch.enableReceive(rfreceive_pin);  //RF433 Mhz
  irrecv.enableIRIn();                    // Enabling IR sensor
  dht.begin();                            // Enabling DHT sensor

  //Blynk.begin(auth, ssid, pass);
  WiFi.begin(ssid, pass);
  Blynk.config(auth);
  timer.setInterval(2000L, checkBlynkStatus);  // check if Blynk server is connected every 2 seconds
  timer.setInterval(1000L, readSensor);        // Sending Sensor Data to Blynk Cloud every 1 second
  delay(1000);

  if (!fetch_blynk_state) {
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
    Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3);
    Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_4);
  }
}

// When App button is pushed - switch the state
// App button1
BLYNK_WRITE(VPIN_BUTTON_1) {
  toggleState_1 = param.asInt();
  digitalWrite(RelayPin1, toggleState_1);
}
// App button2
BLYNK_WRITE(VPIN_BUTTON_2) {
  toggleState_2 = param.asInt();
  digitalWrite(RelayPin2, toggleState_2);
}
// App button3
BLYNK_WRITE(VPIN_BUTTON_3) {
  toggleState_3 = param.asInt();
  digitalWrite(RelayPin3, toggleState_3);
}
// App button4
BLYNK_WRITE(VPIN_BUTTON_4) {
  toggleState_4 = param.asInt();
  digitalWrite(RelayPin4, toggleState_4);
}
// App button5
BLYNK_WRITE(VPIN_BUTTON_OK) {
  all_SwitchOff();
}
// App button5
void all_SwitchOff() {
  toggleState_1 = 0;
  digitalWrite(RelayPin1, LOW);
  Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  delay(100);
  toggleState_2 = 0;
  digitalWrite(RelayPin2, LOW);
  Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
  delay(100);
  toggleState_3 = 0;
  digitalWrite(RelayPin3, LOW);
  Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3);
  delay(100);
  toggleState_4 = 0;
  digitalWrite(RelayPin4, LOW);
  Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_4);
  delay(100);
  Blynk.virtualWrite(VPIN_HUMIDITY, humidity1);
  Blynk.virtualWrite(VPIN_TEMPERATURE, temperature1);
}

// Check blynk status
void checkBlynkStatus() {  // called every 2 seconds by SimpleTimer
  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    wifiFlag = 1;
    Serial.println("Blynk Not Connected");
    digitalWrite(wifiLed, LOW);
  }
  if (isconnected == true) {
    wifiFlag = 0;
    if (!fetch_blynk_state) {
      Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
      Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
      Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3);
      Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_4);
    }
    digitalWrite(wifiLed, HIGH);
    //Serial.println("Blynk Connected");
  }
}

// Sync the last server values
BLYNK_CONNECTED() {
  // Request the latest state from the server
  if (fetch_blynk_state) {
    Blynk.syncVirtual(VPIN_BUTTON_1);
    Blynk.syncVirtual(VPIN_BUTTON_2);
    Blynk.syncVirtual(VPIN_BUTTON_3);
    Blynk.syncVirtual(VPIN_BUTTON_4);
  }
  Blynk.syncVirtual(VPIN_TEMPERATURE);
  Blynk.syncVirtual(VPIN_HUMIDITY);
}

// Read sensor data
void readSensor() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  } else {
    humidity1 = h;
    temperature1 = t;
    Blynk.virtualWrite(VPIN_HUMIDITY, humidity1);
    Blynk.virtualWrite(VPIN_TEMPERATURE, temperature1);
    // Serial.println(temperature1);
    // Serial.println(humidity1);
  }
}

//IR Remote
void ir_remote() {
  if (irrecv.decode(&results)) {
    switch (results.value) {
      case IR_Button_1:
        digitalWrite(RelayPin1, toggleState_1);
        toggleState_1 = !toggleState_1;
        Blynk.virtualWrite(VPIN_BUTTON_1, !toggleState_1);
        delay(100);
        break;
      case IR_Button_2:
        digitalWrite(RelayPin2, toggleState_2);
        toggleState_2 = !toggleState_2;
        Blynk.virtualWrite(VPIN_BUTTON_2, !toggleState_2);
        delay(100);
        break;
      case IR_Button_3:
        digitalWrite(RelayPin3, toggleState_3);
        toggleState_3 = !toggleState_3;
        Blynk.virtualWrite(VPIN_BUTTON_3, !toggleState_3);
        delay(100);
        break;
      case IR_Button_4:
        digitalWrite(RelayPin4, toggleState_4);
        toggleState_4 = !toggleState_4;
        Blynk.virtualWrite(VPIN_BUTTON_4, !toggleState_4);
        delay(100);
        break;
      case IR_All_Off:
        all_SwitchOff();
        break;
      default: break;
    }
    //Serial.println(results.value, HEX);
    irrecv.resume();
  }
}

//RF433 Mhz Remote
void remoterf433_control() {
  if (mySwitch.available()) {
    int value = mySwitch.getReceivedValue();
    if (value == 14527912) {
      digitalWrite(RelayPin1, toggleState_1);
      toggleState_1 = !toggleState_1;
      Blynk.virtualWrite(VPIN_BUTTON_1, !toggleState_1);
    } else if (value == 16736113) {
      digitalWrite(RelayPin2, toggleState_2);
      toggleState_2 = !toggleState_2;
      Blynk.virtualWrite(VPIN_BUTTON_2, !toggleState_2);
    } else if (value == 16736114) {
      digitalWrite(RelayPin3, toggleState_3);
      toggleState_3 = !toggleState_3;
      Blynk.virtualWrite(VPIN_BUTTON_3, !toggleState_3);
    } else if (value == 16736120) {
      digitalWrite(RelayPin4, toggleState_4);
      toggleState_4 = !toggleState_4;
      Blynk.virtualWrite(VPIN_BUTTON_4, !toggleState_4);
    }
    mySwitch.resetAvailable();
  }
}


void manual_control() {
  if (digitalRead(SwitchPin1) == LOW && SwitchState_1 == LOW) {
    digitalWrite(RelayPin1, LOW);
    toggleState_1 = HIGH;
    SwitchState_1 = HIGH;
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    Serial.println("Switch-1 on");
  }
  if (digitalRead(SwitchPin1) == HIGH && SwitchState_1 == HIGH) {
    digitalWrite(RelayPin1, HIGH);
    toggleState_1 = LOW;
    SwitchState_1 = LOW;
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    Serial.println("Switch-1 off");
  }
  if (digitalRead(SwitchPin2) == LOW && SwitchState_2 == LOW) {
    digitalWrite(RelayPin2, LOW);
    toggleState_2 = HIGH;
    SwitchState_2 = HIGH;
    Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
    Serial.println("Switch-2 on");
  }
  if (digitalRead(SwitchPin2) == HIGH && SwitchState_2 == HIGH) {
    digitalWrite(RelayPin2, HIGH);
    toggleState_2 = LOW;
    SwitchState_2 = LOW;
    Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
    Serial.println("Switch-2 off");
  }
  if (digitalRead(SwitchPin3) == LOW && SwitchState_3 == LOW) {
    digitalWrite(RelayPin3, LOW);
    toggleState_3 = HIGH;
    SwitchState_3 = HIGH;
    Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3);
    Serial.println("Switch-3 on");
  }
  if (digitalRead(SwitchPin3) == HIGH && SwitchState_3 == HIGH) {
    digitalWrite(RelayPin3, HIGH);
    toggleState_3 = LOW;
    SwitchState_3 = LOW;
    Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3);
    Serial.println("Switch-3 off");
  }
  if (digitalRead(SwitchPin4) == LOW && SwitchState_4 == LOW) {
    digitalWrite(RelayPin4, LOW);
    toggleState_4 = HIGH;
    SwitchState_4 = HIGH;
    Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_4);
    Serial.println("Switch-4 on");
  }
  if (digitalRead(SwitchPin4) == HIGH && SwitchState_4 == HIGH) {
    digitalWrite(RelayPin4, HIGH);
    toggleState_4 = LOW;
    SwitchState_4 = LOW;
    Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_4);
    Serial.println("Switch-4 off");
  }
}

// Loop function
void loop() {
  //manual_control();
  ir_remote();  //IR remote Control
  remoterf433_control();
  Blynk.run();
  timer.run();
}
