/*************************************************************************************************
    ESP32+PZEM017+2CH Relay to Nextion,GGSheet and Blynk 2.0
    Update send sensor data to Blynk IoT 
    Created By: Sompoch Tongnamtiang
    Created On: 13 Apr, 2023
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
//==========IO connect to relay===========//
#define sw1 18  //Map to Blynk virtual pin V6
#define sw2 19  //Set Charge/Discharge
//==========IO connect to relay===========//
uint32_t valuesetpointv;
//==================Modbus================//
#include <ModbusMaster.h>
#define RX2 16  //RX
#define TX2 17  //TX
// instantiate ModbusMaster object
ModbusMaster node1;  //Slave ID1 PZEM017
//==================Modbus================//

//================Pzem017=================//
static uint8_t pzemSlaveAddr = 0x01;
static uint16_t NewshuntAddr = 0x0000;  // shunt. Default 0x0000 is 100A, replace to "0x0001" if using 50A shunt, 0x0002 is for 200A, 0x0003 is for 300A
float PZEMVoltage = 0;
float PZEMCurrent = 0;
float PZEMPower = 0;
float PZEMEnergy = 0;
unsigned long startMillisPZEM;
unsigned long currentMillisPZEM;
const unsigned long periodPZEM = 1500;
unsigned long startMillisReadData;
int Reset_Energy = 0;
int a = 1;
unsigned long startMillis1;
//================Pzem017=================//

//==============SimpleTimer===============//
#include <SimpleTimer.h>
SimpleTimer timer;
//==============SimpleTimer===============//

//======Nextion Library and Object========//
#include "Nextion.h"  //Nextion libraries
// Nextion Objects, ada 5 tombol
// (page id, component id, component name)
//Page 3 Pzem017 Data
NexText texvolt = NexText(3, 5, "texvolt");
NexText texcurrent = NexText(3, 6, "texcurrent");
NexText texpower = NexText(3, 8, "texpower");
NexText texenergy = NexText(3, 7, "texenergy");
//Page id 4 Voltage Setpoint
//Voltage
NexText textvolt2 = NexText(4, 5, "textvolt2");
NexNumber nsetpointv = NexNumber(4, 6, "nsetpointv");
NexNumber nvalsetpointv = NexNumber(4, 7, "nvalsetpointv");
NexText tstate1 = NexText(4, 8, "tstate1");
NexText textvolt1 = NexText(4, 9, "textvolt1");
//Page 5 Dual State button
NexButton bt0 = NexButton(5, 2, "bt0");
// Declare variable global
bool statusbt0 = false;
//bool statusbt1 = false;
// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
  &bt0,
  &texvolt,
  &texcurrent,
  &texpower,
  &texenergy,
  &textvolt2,
  &textvolt1,
  NULL
};
//======Nextion Library and Object========//

//=============ON,OFF Relay1==============//
void bt0PushCallback(void *ptr) {
  if (statusbt0 == false) {
    digitalWrite(sw1, LOW);
    statusbt0 = true;
  } else if (statusbt0 == true) {
    digitalWrite(sw1, HIGH);
    statusbt0 = false;
  }
}
//=============ON,OFF Relay1==============//
//==============Setup Function============//
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N2, RX2, TX2);  //RX2=16,RO ,TX2=17, DI
  startMillisPZEM = millis();
  node1.begin(pzemSlaveAddr, Serial2);  //Pzem017-Battery
  delay(1000);
  startMillisReadData = millis();

  nexInit();

  // Register the push/pop event callback function
  bt0.attachPush(bt0PushCallback, &bt0);

  // Set IO pinMode for relay
  pinMode(sw1, OUTPUT);
  pinMode(sw2, OUTPUT);

  //Set default relay status at boot
  digitalWrite(sw1, HIGH);
  digitalWrite(sw2, HIGH);

  timer.setInterval(3000L, Pzem017toNex);
}
//==============Setup Function============//

//============Read Pzem017================//
void Pzem017toNex() {
  {
    if ((millis() - startMillis1 >= 10000) && (a == 1)) {
      setShunt(pzemSlaveAddr);
      a = 0;
    }
    currentMillisPZEM = millis();
    if (currentMillisPZEM - startMillisPZEM >= periodPZEM) {
      uint8_t result;
      result = node1.readInputRegisters(0x0000, 6);
      if (result == node1.ku8MBSuccess) {
        uint32_t tempdouble = 0x00000000;
        PZEMVoltage = node1.getResponseBuffer(0x0000) / 100.0;
        PZEMCurrent = node1.getResponseBuffer(0x0001) / 100.0;
        tempdouble = (node1.getResponseBuffer(0x0003) << 16) + node1.getResponseBuffer(0x0002);
        PZEMPower = tempdouble / 10.0;
        tempdouble = (node1.getResponseBuffer(0x0005) << 16) + node1.getResponseBuffer(0x0004);
        PZEMEnergy = tempdouble / 1000.00;  //หน่วยเป็น KWh
        Serial.print("Voltage : ");
        Serial.print(PZEMVoltage);
        Serial.println(" V ");
        Serial.print("Current : ");
        Serial.print(PZEMCurrent);
        Serial.println(" A ");
        Serial.print("Power : ");
        Serial.print(PZEMPower);
        Serial.println(" W ");
        Serial.print("Energy : ");
        Serial.print(PZEMEnergy);
        Serial.println(" KWh ");
      }
      startMillisReadData = millis();
    }

    String command = "texvolt.txt=\"" + String(PZEMVoltage) + "\"";
    Serial.print(command);
    endNextionCommand();

    String command1 = "texvolt.txt=\"" + String(PZEMVoltage) + "\"";
    Serial.print(command1);
    endNextionCommand();

    String command2 = "texcurrent.txt=\"" + String(PZEMCurrent) + "\"";
    Serial.print(command2);
    endNextionCommand();

    String command3 = "texpower.txt=\"" + String(PZEMPower) + "\"";
    Serial.print(command3);
    endNextionCommand();

    String command4 = "texenergy.txt=\"" + String(PZEMEnergy) + "\"";
    Serial.print(command4);
    endNextionCommand();

    //Set point
    String command5 = "textvolt1.txt=\"" + String(PZEMVoltage) + "\"";
    Serial.print(command5);
    endNextionCommand();

    String command6 = "textvolt2.txt=\"" + String(PZEMVoltage) + "\"";
    Serial.print(command6);
    endNextionCommand();

    nsetpointv.getValue(&valuesetpointv);
    endNextionCommand();

    nvalsetpointv.setValue(valuesetpointv);
    endNextionCommand();

    nsetpointv.getValue(&valuesetpointv);
    endNextionCommand();

    nvalsetpointv.setValue(valuesetpointv);
    endNextionCommand();

    if (PZEMVoltage < valuesetpointv) {
      digitalWrite(sw2, LOW);
      tstate1.setText("CHARGE");
    } else {
      digitalWrite(sw2, HIGH);
      tstate1.setText("DISCHARGE");
    }
  }
}
//============Read Pzem017================//

//===========endNextionCommand============//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//===========endNextionCommand============//

//==========Set Shunt address=============//
void setShunt(uint8_t slaveAddr) {
  static uint8_t SlaveParameter = 0x06;
  static uint16_t registerAddress = 0x0000;
  uint16_t u16CRC = 0xFFFF;
  u16CRC = crc16_update(u16CRC, slaveAddr);
  u16CRC = crc16_update(u16CRC, SlaveParameter);
  u16CRC = crc16_update(u16CRC, highByte(registerAddress));
  u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
  u16CRC = crc16_update(u16CRC, highByte(NewshuntAddr));
  u16CRC = crc16_update(u16CRC, lowByte(NewshuntAddr));
  Serial2.write(slaveAddr);
  Serial2.write(SlaveParameter);
  Serial2.write(highByte(registerAddress));
  Serial2.write(lowByte(registerAddress));
  Serial2.write(highByte(NewshuntAddr));
  Serial2.write(lowByte(NewshuntAddr));
  Serial2.write(lowByte(u16CRC));
  Serial2.write(highByte(u16CRC));
  delay(10);
  delay(100);
}
//==========Set Shunt address=============//

//=============Loop Function==============//
void loop() {
  //When push/pop event occured execute component in touch event list
  nexLoop(nex_listen_list);

  timer.run();
}
//=============Loop Function==============//
