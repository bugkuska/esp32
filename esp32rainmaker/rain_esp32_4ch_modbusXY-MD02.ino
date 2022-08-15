//===========Modbus Object=========//
#include <ModbusMaster.h>
#define RX2             16    //RO
#define TX2             17    //DI
ModbusMaster node1;
//===========Modbus Object=========//

//===========Rain Maker============//
#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include <SimpleTimer.h>
SimpleTimer Timer;
#include <wifi_provisioning/manager.h>
//===========Rain Maker============//

//=======Set Defalt Values=========//
#define DEFAULT_RELAY_MODE false
#define DEFAULT_Temperature 0
#define DEFAULT_Humidity 0
bool wifi_connected = 0;
//=======Set Defalt Values=========//

//=======Set BLE Credentils========//
const char *service_name = "PROV_12345";
const char *pop = "1234567";
//=======Set BLE Credentils========//

//============Set GPIO=============//
static uint8_t gpio_reset = 14;   //Reset button
static uint8_t wifiLed    = 2;
static uint8_t relay1 = 26;
static uint8_t relay2 = 25;
static uint8_t relay3 = 33;
static uint8_t relay4 = 32;
//============Set GPIO=============//

//========Set relay state==========//
//Relay Active HIGH
bool relay1_state = LOW;
bool relay2_state = LOW;
bool relay3_state = LOW;
bool relay4_state = LOW;
//========Set relay state==========//

//======Declaring Devices=========//
//The framework provides some standard device types like switch, lightbulb, fan, temperature sensor.
static TemperatureSensor temperature1("XY-MD02 อุณหภูมิ1");
static TemperatureSensor humidity1("XY-MD02 ความชื้นสัมพัทธ์1");
static Switch my_switch1("relay1", &relay1);
static Switch my_switch2("relay2", &relay2);
static Switch my_switch3("relay3", &relay3);
static Switch my_switch4("relay4", &relay4);
//======Declaring Devices=========//

//==========sysProvEvent===========//
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
      wifi_connected = 1;
      digitalWrite(wifiLed, HIGH);
      delay(500);
      break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
        break;
      }
    case ARDUINO_EVENT_PROV_INIT:
      wifi_prov_mgr_disable_auto_stop(10000);
      break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      Serial.println("Stopping Provisioning!!!");
      wifi_prov_mgr_stop_provisioning();
      break;
  }
}
//==========sysProvEvent===========//

//=========Write Callback==========//
void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx)
{
  const char *device_name = device->getDeviceName();
  Serial.println(device_name);
  const char *param_name = param->getParamName();
  //Relay1
  if (strcmp(device_name, "relay1") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      relay1_state = val.val.b;
      (relay1_state == false) ? digitalWrite(relay1, LOW) : digitalWrite(relay1, HIGH);
      param->updateAndReport(val);
    }
  }
  //Relay2
  else if (strcmp(device_name, "relay2") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      relay2_state = val.val.b;
      (relay2_state == false) ? digitalWrite(relay2, LOW) : digitalWrite(relay2, HIGH);
      param->updateAndReport(val);
    }
  }
  //Relay3
  else if (strcmp(device_name, "relay3") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      relay3_state = val.val.b;
      (relay3_state == false) ? digitalWrite(relay3, LOW) : digitalWrite(relay3, HIGH);
      param->updateAndReport(val);
    }
  }
  //Relay4
  else if (strcmp(device_name, "relay4") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      relay4_state = val.val.b;
      (relay4_state == false) ? digitalWrite(relay4, LOW) : digitalWrite(relay4, HIGH);
      param->updateAndReport(val);
    }
  }
}
//=========Write Callback==========//

//=========Setup Function==========//
void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); //RX2=16,RO ,TX2=17, DI
  node1.begin(1, Serial2);

  // Configure the input GPIOs
  pinMode(gpio_reset, INPUT_PULLUP);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(wifiLed, OUTPUT);

  digitalWrite(relay1, DEFAULT_RELAY_MODE);
  digitalWrite(relay2, DEFAULT_RELAY_MODE);
  digitalWrite(relay3, DEFAULT_RELAY_MODE);
  digitalWrite(relay4, DEFAULT_RELAY_MODE);
  digitalWrite(wifiLed, LOW);

  //============Declaring Node==========//
  Node my_node;
  my_node = RMaker.initNode("SMFThailand");
  //Standard switch device
  my_switch1.addCb(write_callback);
  my_switch2.addCb(write_callback);
  my_switch3.addCb(write_callback);
  my_switch4.addCb(write_callback);
  //============Declaring Node==========//

  //=======Adding Devices in Node=======//
  my_node.addDevice(temperature1);
  my_node.addDevice(humidity1);
  my_node.addDevice(my_switch1);
  my_node.addDevice(my_switch2);
  my_node.addDevice(my_switch3);
  my_node.addDevice(my_switch4);
  //=======Adding Devices in Node=======//

  //This is optional
  RMaker.enableOTA(OTA_USING_PARAMS);
  //If you want to enable scheduling, set time zone for your region using setTimeZone().
  //The list of available values are provided here https://rainmaker.espressif.com/docs/time-service.html
  // RMaker.setTimeZone("Asia/Shanghai");
  // Alternatively, enable the Timezone service and let the phone apps set the appropriate timezone
  RMaker.enableTZService();
  RMaker.enableSchedule();

  Serial.printf("\nStarting ESP-RainMaker\n");
  RMaker.start();

  // Timer for Sending Sensor's Data
  Timer.setInterval(3000);

  WiFi.onEvent(sysProvEvent);

#if CONFIG_IDF_TARGET_ESP32
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif

  my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);
  my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);
  my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);
  my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);
}
//=========Setup Function==========//

//==========Loop Function==========//
void loop() {
  // Check is ready a second timer
  if (Timer.isReady() && wifi_connected) {                    
    Serial.println("Sending Sensor's Data");
    Send_Sensor();      // dl13();
    Timer.reset();      // Reset a second timer
  }

  //Logic to Reset RainMaker
  // Read GPIO0 (external button to reset device
  if (digitalRead(gpio_reset) == LOW)   //Push button pressed
  { 
    Serial.printf("Reset Button Pressed!\n");
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(gpio_reset) == LOW) delay(50);
    int endTime = millis();

    if ((endTime - startTime) > 10000) {
      // If key pressed for more than 10secs, reset all
      Serial.printf("Reset to factory.\n");
      wifi_connected = 0;
      RMakerFactoryReset(2);
    } else if ((endTime - startTime) > 3000) {
      Serial.printf("Reset Wi-Fi.\n");
      wifi_connected = 0;
      // If key pressed for more than 3secs, but less than 10, reset Wi-Fi
      RMakerWiFiReset(2);
    }
  }
  delay(100);

  if (WiFi.status() != WL_CONNECTED)
  {
    //Serial.println("WiFi Not Connected");
    digitalWrite(wifiLed, LOW);
  }
  else
  {
    //Serial.println("WiFi Connected");
    digitalWrite(wifiLed, HIGH);
  }
}
//==========Loop Function==========//

//========Read Sensor Data=========//
void Send_Sensor()
{
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float humi1 = (node1.getResponseBuffer(1) / 10.0f);
  Serial.println("Get XY-MD02-Sensor1");
  result1 = node1.readInputRegisters(0x0001, 2); // Read 2 registers starting at 1)
  if (result1 == node1.ku8MBSuccess)
  {
    Serial.print("Temperature1: ");
    Serial.print(node1.getResponseBuffer(0) / 10.0f);
    Serial.print("Humidity1: ");
    Serial.println(node1.getResponseBuffer(1) / 10.0f);

    temperature1.updateAndReportParam("Temperature", temp1);
    humidity1.updateAndReportParam("Temperature", humi1);
  }
}
//========Read Sensor Data=========//
