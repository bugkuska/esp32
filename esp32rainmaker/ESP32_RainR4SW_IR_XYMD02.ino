/*************************************************************************************************
    Source from
    Created By: Tauseef Ahmad
    Created On: 28 July, 2022

    YouTube Video: https://youtu.be/inVhQaKLdlk
    My Channel: https://www.youtube.com/channel/UCOXYfOHgu-C-UfGyDcu5sYw/

    Update Read Modbus Temperature and Humidity
    Created By: Sompoch Tongnamtiang
    Created On: 20 Sep, 2022
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1. ezButton
    2. IRremote
    3. Modbus-Master
    4. SimpleTimer
 ***********************************************************************************************/
//===========Modbus Object=========//
#include <ModbusMaster.h>
#define RX2             16    //RO
#define TX2             17    //DI
ModbusMaster node1;           //XY-MD02
#define DEFAULT_Temperature 0
#define DEFAULT_Humidity 0
//===========Modbus Object=========//

//==========Simple Timer===========//
#include <SimpleTimer.h>
SimpleTimer timer;
//==========Simple Timer===========//

//===========RainMaker=============//
#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include <EEPROM.h>
//===========RainMaker=============//

//---------------------------------------------------
//install these libraries
#include <ezButton.h>
#include <IRremote.h>
//---------------------------------------------------
const char *service_name = "PROV_12345";
const char *pop = "1234567";
//---------------------------------------------------
#define EEPROM_SIZE 4
const byte IR_RECEIVE_PIN = 14;
//---------------------------------------------------
// define the Device Names
char device1[] = "Switch1";
char device2[] = "Switch2";
char device3[] = "Switch3";
char device4[] = "Switch4";
//---------------------------------------------------
// define the GPIO connected with Relays and switches
static uint8_t RELAY_1 = 25;
static uint8_t RELAY_2 = 26;
static uint8_t RELAY_3 = 33;
static uint8_t RELAY_4 = 32;
//---------------------------------------------------
ezButton button1(34);
ezButton button2(35);
ezButton button3(36);
ezButton button4(39);
//---------------------------------------------------
static uint8_t WIFI_LED    = 2;
static uint8_t gpio_reset = 0;
//---------------------------------------------------
// Relay State
bool STATE_RELAY_1 = LOW;
bool STATE_RELAY_2 = LOW;
bool STATE_RELAY_3 = LOW;
bool STATE_RELAY_4 = LOW;
//---------------------------------------------------
static TemperatureSensor temperature1("XY-MD02 อุณหภูมิ");
static TemperatureSensor humidity1("XY-MD02 ความชื้นสัมพัทธ์");
//The framework provides some standard device types
//like switch, lightbulb, fan, temperature sensor.
static Switch my_switch1(device1, &RELAY_1);
static Switch my_switch2(device2, &RELAY_2);
static Switch my_switch3(device3, &RELAY_3);
static Switch my_switch4(device4, &RELAY_4);
//---------------------------------------------------
/****************************************************************************************************
   sysProvEvent Function
*****************************************************************************************************/
void sysProvEvent(arduino_event_t *sys_event)
{
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
      printQR(service_name, pop, "ble");
#else
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", service_name, pop);
      printQR(service_name, pop, "softap");
#endif
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.printf("\nConnected to Wi-Fi!\n");
      digitalWrite(WIFI_LED, HIGH);
      break;
  }
}

/****************************************************************************************************
   write_callback Function
*****************************************************************************************************/
void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx)
{
  const char *device_name = device->getDeviceName();
  const char *param_name = param->getParamName();
  //----------------------------------------------------------------------------------
  if (strcmp(device_name, device1) == 0) {

    Serial.printf("Lightbulb1 = %s\n", val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0) {
      //Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
      STATE_RELAY_1 = val.val.b;
      STATE_RELAY_1 = !STATE_RELAY_1;
      control_relay(1, RELAY_1, STATE_RELAY_1);
      (STATE_RELAY_1 == false) ? digitalWrite(RELAY_1, LOW) : digitalWrite(RELAY_1, HIGH);
      param->updateAndReport(val);
    }
  }
  //----------------------------------------------------------------------------------
  else if (strcmp(device_name, device2) == 0) {

    Serial.printf("Switch value = %s\n", val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0) {
      //Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
      STATE_RELAY_2 = val.val.b;
      STATE_RELAY_2 = !STATE_RELAY_2;
      control_relay(2, RELAY_2, STATE_RELAY_2);
      (STATE_RELAY_2 == false) ? digitalWrite(RELAY_2, LOW) : digitalWrite(RELAY_2, HIGH);
      param->updateAndReport(val);
    }
  }
  //----------------------------------------------------------------------------------
  else if (strcmp(device_name, device3) == 0) {

    Serial.printf("Switch value = %s\n", val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0) {
      //Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
      STATE_RELAY_3 = val.val.b;
      STATE_RELAY_3 = !STATE_RELAY_3;
      control_relay(3, RELAY_3, STATE_RELAY_3);
      (STATE_RELAY_3 == false) ? digitalWrite(RELAY_3, LOW) : digitalWrite(RELAY_3, HIGH);
      param->updateAndReport(val);
    }
  }
  //----------------------------------------------------------------------------------
  else if (strcmp(device_name, device4) == 0) {

    Serial.printf("Switch value = %s\n", val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0) {
      //Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
      STATE_RELAY_4 = val.val.b;
      STATE_RELAY_4 = !STATE_RELAY_4;
      control_relay(4, RELAY_4, STATE_RELAY_4);
      (STATE_RELAY_4 == false) ? digitalWrite(RELAY_4, LOW) : digitalWrite(RELAY_4, HIGH);
      param->updateAndReport(val);
    }
  }
  //----------------------------------------------------------------------------------
}

/****************************************************************************************************
   setup Function
*****************************************************************************************************/
void setup() {
  //------------------------------------------------------------------------------
  uint32_t chipId = 0;
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); //RX2=16,RO ,TX2=17, DI
  node1.begin(1, Serial2);
  //------------------------------------------------------------------------------
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  //------------------------------------------------------------------------------
  IrReceiver.begin(IR_RECEIVE_PIN); // Start the IR receiver
  //------------------------------------------------------------------------------

  // Set the Relays GPIOs as output mode
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  //------------------------------------------------------------------------------
  // set debounce time to 100 milliseconds
  button1.setDebounceTime(100);
  button2.setDebounceTime(100);
  button3.setDebounceTime(100);
  button4.setDebounceTime(100);
  //------------------------------------------------------------------------------
  pinMode(gpio_reset, INPUT);
  pinMode(WIFI_LED, OUTPUT);
  digitalWrite(WIFI_LED, LOW);
  //------------------------------------------------------------------------------
  // Write to the GPIOs the default state on booting
  digitalWrite(RELAY_1, !STATE_RELAY_1);
  digitalWrite(RELAY_2, !STATE_RELAY_2);
  digitalWrite(RELAY_3, !STATE_RELAY_3);
  digitalWrite(RELAY_4, !STATE_RELAY_4);
  //------------------------------------------------------------------------------
  Node my_node;
  my_node = RMaker.initNode("xESP32R4SW_IR_XYMD02x");
  //------------------------------------------------------------------------------
  //Standard switch device
  my_switch1.addCb(write_callback);
  my_switch2.addCb(write_callback);
  my_switch3.addCb(write_callback);
  my_switch4.addCb(write_callback);
  //------------------------------------------------------------------------------
  //Add switch device to the node
  my_node.addDevice(temperature1);
  my_node.addDevice(humidity1);

  my_node.addDevice(my_switch1);
  my_node.addDevice(my_switch2);
  my_node.addDevice(my_switch3);
  my_node.addDevice(my_switch4);
  //------------------------------------------------------------------------------
  //This is optional
  RMaker.enableOTA(OTA_USING_PARAMS);
  //If you want to enable scheduling, set time zone for your region using setTimeZone().
  //The list of available values are provided here https://rainmaker.espressif.com/docs/time-service.html
  RMaker.setTimeZone("Asia/Bangkok");
  // Alternatively, enable the Timezone service and let the phone apps set the appropriate timezone
  RMaker.enableTZService();
  RMaker.enableSchedule();
  //------------------------------------------------------------------------------
  //Service Name
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  Serial.printf("\nChip ID:  %d Service Name: %s\n", chipId, service_name);
  //------------------------------------------------------------------------------
  Serial.printf("\nStarting ESP-RainMaker\n");
  RMaker.start();
  //------------------------------------------------------------------------------
  // Timer for Sending Sensor's Data
  timer.setInterval(10000L, Send_Sensor);

  WiFi.onEvent(sysProvEvent);
#if CONFIG_IDF_TARGET_ESP32
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif
  //------------------------------------------------------------------------------
  STATE_RELAY_1 = EEPROM.read(0);
  STATE_RELAY_2 = EEPROM.read(1);
  STATE_RELAY_3 = EEPROM.read(2);
  STATE_RELAY_4 = EEPROM.read(3);

  digitalWrite(RELAY_1, STATE_RELAY_1);
  digitalWrite(RELAY_2, STATE_RELAY_2);
  digitalWrite(RELAY_3, STATE_RELAY_3);
  digitalWrite(RELAY_4, STATE_RELAY_4);

  my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_1);
  my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_2);
  my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_3);
  my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_4);

  Serial.printf("Relay1 is %s \n", STATE_RELAY_1 ? "ON" : "OFF");
  Serial.printf("Relay2 is %s \n", STATE_RELAY_2 ? "ON" : "OFF");
  Serial.printf("Relay3 is %s \n", STATE_RELAY_3 ? "ON" : "OFF");
  Serial.printf("Relay4 is %s \n", STATE_RELAY_4 ? "ON" : "OFF");
  //------------------------------------------------------------------------------
}

//========Read Sensor Data=========//
void Send_Sensor()
{
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float humi1 = (node1.getResponseBuffer(1) / 10.0f);
  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2); //Function 04, Read 2 registers starting at 2)
  if (result1 == node1.ku8MBSuccess)
  {
    Serial.print("Temp: ");
    Serial.println(node1.getResponseBuffer(0) / 10.0f);
    Serial.print("Humi: ");
    Serial.println(node1.getResponseBuffer(1) / 10.0f);

    temperature1.updateAndReportParam("Temperature", temp1);
    humidity1.updateAndReportParam("Temperature", humi1);
  }
}
//========Read Sensor Data=========//
/****************************************************************************************************
   loop Function
*****************************************************************************************************/
void loop()
{
  timer.run();
  //------------------------------------------------------------------------------
  // Read GPIO0 (external button to reset device
  if (digitalRead(gpio_reset) == LOW) { //Push button pressed
    Serial.printf("Reset Button Pressed!\n");
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(gpio_reset) == LOW) delay(50);
    int endTime = millis();
    //_______________________________________________________________________
    if ((endTime - startTime) > 10000) {
      // If key pressed for more than 10secs, reset all
      Serial.printf("Reset to factory.\n");
      RMakerFactoryReset(2);
    }
    //_______________________________________________________________________
    else if ((endTime - startTime) > 3000) {
      Serial.printf("Reset Wi-Fi.\n");
      // If key pressed for more than 3secs, but less than 10, reset Wi-Fi
      RMakerWiFiReset(2);
    }
    //_______________________________________________________________________
  }
  //------------------------------------------------------------------------------
  delay(100);

  if (WiFi.status() != WL_CONNECTED) {
    //Serial.println("WiFi Not Connected");
    digitalWrite(WIFI_LED, LOW);
  }
  else {
    //Serial.println("WiFi Connected");
    digitalWrite(WIFI_LED, HIGH);
  }
  //------------------------------------------------------------------------------
  button_control();
  remoteir_control();
}

/*******************************************************************************
   button_control function:
 ******************************************************************************/
void button_control() {
  button1.loop();
  if (button1.isPressed()) {
    control_relay(1, RELAY_1, STATE_RELAY_1);
    my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_1);
  }
  button2.loop();
  if (button2.isPressed()) {
    control_relay(2, RELAY_2, STATE_RELAY_2);
    my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_2);
  }
  button3.loop();
  if (button3.isPressed()) {
    control_relay(3, RELAY_3, STATE_RELAY_3);
    my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_3);
  }
  button4.loop();
  if (button4.isPressed()) {
    control_relay(4, RELAY_4, STATE_RELAY_4);
    my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_4);
  }
}

/****************************************************************************************************
   control_relay Function
*****************************************************************************************************/
void control_relay(int relay_no, int relay_pin, boolean &status) {
  status = !status;
  digitalWrite(relay_pin, status);
  EEPROM.write(relay_no - 1, status);
  EEPROM.commit();
  String text = (status) ? "ON" : "OFF";
  Serial.println("Relay" + String(relay_no) + " is " + text);
}

/****************************************************************************************************
   remote_control Function
*****************************************************************************************************/
void remoteir_control()
{
  if (IrReceiver.decode()) {
    String ir_code = String(IrReceiver.decodedIRData.command, HEX);
    if (ir_code.equals("0")) {
      IrReceiver.resume();
      return;
    }

    Serial.println(ir_code);
    //Chang IR code value & Generate from ir-code.ino
    if (ir_code == "16") {
      control_relay(1, RELAY_1, STATE_RELAY_1);
      my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_1);
    }
    else if (ir_code == "19") {
      control_relay(2, RELAY_2, STATE_RELAY_2);
      my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_2);
    }
    else if (ir_code == "d") {
      control_relay(3, RELAY_3, STATE_RELAY_3);
      my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_3);
    }
    else if (ir_code == "c") {
      control_relay(4, RELAY_4, STATE_RELAY_4);
      my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, STATE_RELAY_4);
    }

    IrReceiver.resume();
  }
}
