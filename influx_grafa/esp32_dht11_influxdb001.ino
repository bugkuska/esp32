#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// WiFi AP SSID
#define WIFI_SSID "" //ชื่อ Wi-Fi
// WiFi password
#define WIFI_PASSWORD "" //รหัสผ่าน Wi-Fi

#define INFLUXDB_URL ""        //http://IP-Influxdb:8086
#define INFLUXDB_TOKEN ""      //Influxdb token
#define INFLUXDB_ORG ""        //Influxdb organization 
#define INFLUXDB_BUCKET ""     //Influxdb Bucket

// Time zone info
#define TZ_INFO "UTC7"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare Data point
Point sensor("wifi_status");

//DHT11
#include "DHT.h"
DHT dht(19, DHT11);  //DHT and BMP sensor parameters
int temp = 0;        //Variables to store sensor readings
int humid = 0;

void setup() {
  Serial.begin(9600);
  //DHT11
  dht.begin();

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Accurate time is necessary for certificate validation and writing in batches
  // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "time.navy.mi.th", "pool.ntp.org", "time.nis.gov");


  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Add tags to the data point
  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WiFi.SSID());
}


void loop() {
  // Clear fields for reusing the point. Tags will remain the same as set above.

  temp = dht.readTemperature();  //Record temperature
  humid = dht.readHumidity();    //Record temperature

  sensor.clearFields();  //Clear fields for reusing the point. Tags will remain untouched

  sensor.addField("temperature", temp);  // Store measured value into point
  sensor.addField("humidity", humid);


  // Store measured value into point
  // Report RSSI of currently connected network
  sensor.addField("rssi", WiFi.RSSI());

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  // Check WiFi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  Serial.println("Waiting 1 second");
  delay(1000);
  // ... code from Write Data step

  // Query will find the RSSI values for last minute for each connected WiFi network with this device
  String query = "from(bucket: \"esp32_dht22\")\n\
   |> range(start: -1m)\n\
   |> filter(fn: (r) => r._measurement == \"wifi_status\" and r._field == \"rssi\")";

  // Print composed query
  Serial.println("Querying for RSSI values written to the \"esp32_dht22\" bucket in the last 1 min... ");
  Serial.println(query);

  // Send query to the server and get result
  FluxQueryResult result = client.query(query);

  Serial.println("Results : ");
  // Iterate over rows.
  while (result.next()) {
    // Get converted value for flux result column 'SSID'
    String ssid = result.getValueByName("SSID").getString();
    Serial.print("SSID '");
    Serial.print(ssid);

    Serial.print("' with RSSI ");
    // Get value of column named '_value'
    long value = result.getValueByName("_value").getLong();
    Serial.print(value);

    // Get value for the _time column
    FluxDateTime time = result.getValueByName("_time").getDateTime();

    String timeStr = time.format("%F %T");

    Serial.print(" at ");
    Serial.print(timeStr);

    Serial.println();
  }

  // Report any error
  if (result.getError() != "") {
    Serial.print("Query result error: ");
    Serial.println(result.getError());
  }

  // Close the result
  result.close();

  Serial.println("==========");

  delay(5000);
  Serial.print("อุณหภูมิ ");
  Serial.print(temp);
  Serial.print(" °C    ความชื้น ");
  Serial.print(humid);
  Serial.println(" %");
}
