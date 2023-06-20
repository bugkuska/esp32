bool fetch_blynk_state = true;  //true or false
//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = ""; //เปลี่ยน Token
//==============Modbus-Master==============//

//=========Your WiFi credentials.=========//
// Set password to "" for open networks.
char ssid[] = "";  //2.4 GHz
char pass[] = "";
int wifiFlag = 0;
#define wifiLed 2  //D2
//=========Your WiFi credentials.=========//

//============Wi-Fi Connection===========//
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
//============Wi-Fi Connection===========//

//============Blynk Connected============//
BLYNK_CONNECTED() {
  // Request the latest state from the server
  if (fetch_blynk_state) {
    digitalWrite(wifiLed, HIGH);
    Blynk.syncAll();
  }
}
//============Blynk Connected============//

//============Setup Function=============//
void setup() {
  Serial.begin(9600);
  // Set IO pinMode for relay
  pinMode(wifiLed, OUTPUT);
  //Set default relay status at boot
  digitalWrite(wifiLed, LOW);
  delay(500);

  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  //Blynk.config(auth);
  Blynk.begin(auth, ssid, pass, IPAddress(128,199,150,59), 8080);
  delay(1000);
}
//============Setup Function=============//

//=============Loop Function=============//
void loop() {
  Blynk.run();
}
//=============Loop Function=============//
