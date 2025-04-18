/*************************************************************************************************
    ESP32+PZEM016+3IN1 Temp, Humi and Lux sensor
    Update send sensor data to Blynk IoT 
    Created By: Sompoch Tongnamtiang
    Created On: 26 JAN, 2023
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
    TX >> RX0
  *Note : ระหว่าง upload source-code ให้ถอดสาย RX0 ออกจากบอร์ด ESP32 พอ upload source-code ค่อยต่อเข้าไปใหม่
    Hoo ya, have fun....
 *************************************************************************************************/
#include "Nextion.h"  //Nextion libraries
//=======Declare Nextion Object===========//
//page id:x, component id:x, component name:x
NexText txtvolt = NexText(1, 2, "txtvolt");
NexText txtcurrent = NexText(1, 3, "txtcurrent");
NexText txthz = NexText(1, 4, "txthz");
NexText txtpower = NexText(1, 5, "txtpower");
NexText txtenergy = NexText(1, 6, "txtenergy");
NexText txtmbhumi = NexText(2, 2, "txtmbhumi");
NexText txtmbtemp = NexText(2, 3, "txtmbtemp");
NexText txtmblux = NexText(2, 4, "txtmblux");
NexGauge guagehumi = NexGauge(2, 5, "guagehumi");
NexGauge guagetemp = NexGauge(2, 6, "guagetemp");
//=======Declare Nextion Object===========//
/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_AUTH_TOKEN ""
#define BLYNK_FIRMWARE_VERSION "0.1.0"
bool fetch_blynk_state = true;  //true or false
//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = BLYNK_AUTH_TOKEN;
//=====Your WiFi credentials.=====//
// Set password to "" for open networks.
char ssid[] = "";  //2.4GHz
char pass[] = "";
int wifiFlag = 0;
#define wifiLed 2  //D2
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//=====Your WiFi credentials.=====//

//========Wi-Fi Connection==========//
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    //delay(1000);
  }
  //while (WiFi.status() != WL_CONNECTED) {
  //  Serial.print('.');
  //  delay(1000);
  // }
  Serial.println(WiFi.localIP());
}
//========Wi-Fi Connection==========//

//===Check Blynk connected Status===//
void checkBlynkStatus() {  // called every 10 seconds by SimpleTimer
  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    wifiFlag = 1;
    Serial.println("Blynk Not Connected");
    digitalWrite(wifiLed, LOW);
    //digitalWrite(sw1, HIGH);
    //digitalWrite(sw2, HIGH);
  }
  if (isconnected == true) {
    wifiFlag = 0;
    if (!fetch_blynk_state) {
      Blynk.syncAll();
      //Blynk.virtualWrite(V1, HIGH);
      //Blynk.virtualWrite(V2, HIGH);
    }
    digitalWrite(wifiLed, HIGH);
    Serial.println("Blynk Connected");
  }
}
//===Check Blynk connected Status===//

//==========Blynk Connected=========//
BLYNK_CONNECTED() {
  // Request the latest state from the server
  if (fetch_blynk_state) {
    Blynk.syncAll();
    // Blynk.syncVirtual(V1);
    // Blynk.syncVirtual(V2);
  }
  //Blynk.syncVirtual(V5);
  //Blynk.syncVirtual(V6);
}
//==========Blynk Connected=========//

//==============Setup Function============//

// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
  &txtvolt,
  &txtcurrent,
  &txthz,
  &txtpower,
  &txtenergy,
  //&guagehumi,
  //&guagetemp,
  //&txtmbhumi,
  //&txtmbtemp,
  // &txtmblux,
  NULL
};

//==================Modbus================//
#include <ModbusMaster.h>
#define RX2 16  //RX
#define TX2 17  //TX
// instantiate ModbusMaster object
ModbusMaster node1;  //Slave ID1 PZEM016
static uint8_t pzemSlaveAddr = 0x01;
int Reset_Energy = 0;
ModbusMaster node2;  //Slave ID2 3IN1
//==================Modbus================//

//==============SimpleTimer===============//
#include <SimpleTimer.h>
SimpleTimer timer;
//==============SimpleTimer===============//

//==============Setup Function============//
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);  //RX2=16,TX2=17
  node1.begin(pzemSlaveAddr, Serial2);
  node2.begin(2, Serial2);
  nexInit();

  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, LOW);

  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  timer.setInterval(5000L, Pzem016);
  timer.setInterval(3000L, mb3in1toNex);
  timer.setInterval(5000L, Pzem016toblynk);
  timer.setInterval(5000L, mb3in1toblynk);
  Blynk.config(auth);
  delay(1000);

  if (!fetch_blynk_state) {
    //Blynk.virtualWrite(V1, HIGH);
    //Blynk.virtualWrite(V2, HIGH);
  }

}
//==============Setup Function============//

/*
BLYNK_WRITE(V7)
{
  if (param.asInt() == 1)
  {
    Reset_Energy = Reset_Energy + 1;
    Serial.println(Reset_Energy);
    if (Reset_Energy > 2) {
      Serial.println("Resetting Energy");
      uint16_t u16CRC = 0xFFFF;
      static uint8_t resetCommand = 0x42;
      uint8_t slaveAddr = pzemSlaveAddr;
      u16CRC = crc16_update(u16CRC, slaveAddr);
      u16CRC = crc16_update(u16CRC, resetCommand);
      //preTransmission1();
      Serial2.write(slaveAddr);
      Serial2.write(resetCommand);
      Serial2.write(lowByte(u16CRC));
      Serial2.write(highByte(u16CRC));
      delay(10);
      //postTransmission1();
      delay(100);
    }
  }
}
*/
void resetEnergy(uint8_t slaveAddr)  //Reset the slave's energy counter
{
  uint16_t u16CRC = 0xFFFF;
  static uint8_t resetCommand = 0x42;
  u16CRC = crc16_update(u16CRC, slaveAddr);
  u16CRC = crc16_update(u16CRC, resetCommand);
  Serial.println("Resetting Energy");
  //preTransmission();
  Serial2.write(slaveAddr);
  Serial2.write(resetCommand);
  Serial2.write(lowByte(u16CRC));
  Serial2.write(highByte(u16CRC));
  delay(10);
  //postTransmission();
  delay(100);
  while (Serial2.available()) {  // Prints the response from the Pzem, do something with it if you like
    Serial.print(char(Serial2.read()), HEX);
    Serial.print(" ");
  }
}

//============Read Pzem016================//
void Pzem016() {
  uint32_t tempdouble = 0x00000000;

  float PZEMVoltage = node1.getResponseBuffer(0x0000) / 10.0;  //get the 16bit value for the voltage, divide it by 10 and cast in the float variable

  tempdouble = (node1.getResponseBuffer(0x0002) << 16) + node1.getResponseBuffer(0x0001);  // Get the 2 16bits registers and combine them to an unsigned 32bit
  float PZEMCurrent = tempdouble / 1000.00;                                                // Divide the unsigned 32bit by 1000 and put in the current float variable

  tempdouble = (node1.getResponseBuffer(0x0004) << 16) + node1.getResponseBuffer(0x0003);
  float PZEMPower = tempdouble / 10.0;

  tempdouble = (node1.getResponseBuffer(0x0006) << 16) + node1.getResponseBuffer(0x0005);
  float PZEMEnergy = tempdouble / 1000.00;

  float PZEMHz = node1.getResponseBuffer(0x0007) / 10.0;

  float PZEMPower_factor = node1.getResponseBuffer(0x0008) / 100.00;

  uint8_t result;

  result = node1.readInputRegisters(0x0000, 9);  //read the 9 registers of the PZEM-014 / 016

  if (result == node1.ku8MBSuccess) {
    /*uint32_t tempdouble = 0x00000000;

    float PZEMVoltage = node1.getResponseBuffer(0x0000) / 10.0;  //get the 16bit value for the voltage, divide it by 10 and cast in the float variable

    tempdouble = (node1.getResponseBuffer(0x0002) << 16) + node1.getResponseBuffer(0x0001);  // Get the 2 16bits registers and combine them to an unsigned 32bit
    float PZEMCurrent = tempdouble / 1000.00;                                              // Divide the unsigned 32bit by 1000 and put in the current float variable

    tempdouble = (node1.getResponseBuffer(0x0004) << 16) + node1.getResponseBuffer(0x0003);
    float PZEMPower = tempdouble / 10.0;

    tempdouble = (node1.getResponseBuffer(0x0006) << 16) + node1.getResponseBuffer(0x0005);
    float PZEMEnergy = tempdouble / 1000.00;

    float PZEMHz = node1.getResponseBuffer(0x0007) / 10.0;
    float PZEMPower_factor = node1.getResponseBuffer(0x0008) / 100.00;
    */
    /*
    Serial.print(PZEMVoltage, 1);  // Print Voltage with 1 decimal
    Serial.print("V   ");

    Serial.print(PZEMHz, 1);
    Serial.print("Hz   ");

    Serial.print(PZEMCurrent, 1);
    Serial.print("A   ");

    Serial.print(PZEMPower, 1);
    Serial.print("W  ");

    Serial.print(PZEMPower_factor, 2);
    Serial.print("pf   ");

    Serial.print(PZEMEnergy, 1);
    Serial.print("Wh  ");
    Serial.println();
    delay(1000);
*/
  }
  String command1 = "txtvolt.txt=\"" + String(PZEMVoltage) + "\"";
  Serial.print(command1);
  //endNextionCommand();
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  String command2 = "txtcurrent.txt=\"" + String(PZEMCurrent) + "\"";
  Serial.print(command2);
  //endNextionCommand();
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  String command3 = "txthz.txt=\"" + String(PZEMHz) + "\"";
  Serial.print(command3);
  //endNextionCommand();
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  String command4 = "txtpower.txt=\"" + String(PZEMPower) + "\"";
  Serial.print(command4);
  //endNextionCommand();
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  String command5 = "txtenergy.txt=\"" + String(PZEMEnergy) + "\"";
  Serial.print(command5);
  //endNextionCommand();
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  //}
  // delay(1000);
}
//============Read Pzem016================//

//==============mb3in1toNex===============//
void mb3in1toNex() {
  //3in1
  uint8_t result1;
  float humi1 = (node2.getResponseBuffer(0) / 10.0f);
  float temp1 = (node2.getResponseBuffer(1) / 10.0f);
  int lux1 = (node2.getResponseBuffer(2));

  result1 = node2.readHoldingRegisters(0x0000, 3);  // Read 2 registers starting at 1)
  if (result1 == node2.ku8MBSuccess) {
  }
  String command1 = "txtmbhumi.txt=\"" + String(humi1) + "\"";
  Serial.print(command1);
  endNextionCommand();

  String command2 = "txtmbtemp.txt=\"" + String(temp1) + "\"";
  Serial.print(command2);
  endNextionCommand();

  String command3 = "txtmblux.txt=\"" + String(lux1) + "\"";
  Serial.print(command3);
  endNextionCommand();

  int val1 = map(humi1, 0, 100, 0, 230);
  Serial.print("guagehumi.val=");  //Send the object tag
  Serial.print(val1);              //Send the value
  endNextionCommand();

  int val2 = map(temp1, 0, 100, 0, 230);
  Serial.print("guagetemp.val=");  //Send the object tag
  Serial.print(val2);              //Send the value
  endNextionCommand();
}
//==============mb3in1toNex===============//

//===========endNextionCommand============//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//===========endNextionCommand============//

//===========sendPZEM016toblynk===========//
void Pzem016toblynk() {
  uint32_t tempdouble = 0x00000000;

  float PZEMVoltage = node1.getResponseBuffer(0x0000) / 10.0;  //get the 16bit value for the voltage, divide it by 10 and cast in the float variable

  tempdouble = (node1.getResponseBuffer(0x0002) << 16) + node1.getResponseBuffer(0x0001);  // Get the 2 16bits registers and combine them to an unsigned 32bit
  float PZEMCurrent = tempdouble / 1000.00;                                                // Divide the unsigned 32bit by 1000 and put in the current float variable

  tempdouble = (node1.getResponseBuffer(0x0004) << 16) + node1.getResponseBuffer(0x0003);
  float PZEMPower = tempdouble / 10.0;

  tempdouble = (node1.getResponseBuffer(0x0006) << 16) + node1.getResponseBuffer(0x0005);
  float PZEMEnergy = tempdouble / 1000.00;

  float PZEMHz = node1.getResponseBuffer(0x0007) / 10.0;

  float PZEMPower_factor = node1.getResponseBuffer(0x0008) / 100.00;

  uint8_t result;

  result = node1.readInputRegisters(0x0000, 9);  //read the 9 registers of the PZEM-014 / 016

  if (result == node1.ku8MBSuccess) {
      Serial.print(PZEMVoltage, 1);  // Print Voltage with 1 decimal
      Serial.print("V   ");

      Serial.print(PZEMHz, 1);
      Serial.print("Hz   ");

      Serial.print(PZEMCurrent, 3);
      Serial.print("A   ");

      Serial.print(PZEMPower, 1);
      Serial.print("W  ");

      Serial.print(PZEMEnergy, 3);
      Serial.print("Wh  ");
      Serial.println();
      
      Blynk.virtualWrite(V1, PZEMVoltage);  //V1 Display Voltage
      Blynk.virtualWrite(V2, PZEMHz);  //V2 Display Hz
      Blynk.virtualWrite(V3, PZEMCurrent);  //V3 Display Current
      Blynk.virtualWrite(V4, PZEMEnergy);    //V4 Energy
      Blynk.virtualWrite(V5, PZEMPower);   //V5 Display Power 
}
  delay(1000);  
}
//===========sendPZEM016toblynk===========//

//============mb3in1toblynk===============//
void mb3in1toblynk() {
  //3in1
  uint8_t result1;
  float humi1 = (node2.getResponseBuffer(0) / 10.0f);
  float temp1 = (node2.getResponseBuffer(1) / 10.0f);
  int lux1 = (node2.getResponseBuffer(2));

  result1 = node2.readHoldingRegisters(0x0000, 3);  // Read 2 registers starting at 1)
  if (result1 == node2.ku8MBSuccess) {
  Blynk.virtualWrite(V6, humi1);
  Blynk.virtualWrite(V7, temp1);
  Blynk.virtualWrite(V8, lux1); 
  }
}
//============mb3in1toblynk===============//

void loop() {
  //When push/pop event occured execute component in touch event list
  nexLoop(nex_listen_list);
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
//==========Loop function===========//
