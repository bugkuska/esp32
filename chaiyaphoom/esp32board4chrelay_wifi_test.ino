#include <WiFi.h>

#define relay1 25
#define relay2 33
#define relay3 32
#define relay4 23

char ssid[] = "smf001";      //ชื่อ SSID ที่เราต้องการเชื่อมต่อ
char pass[] = "0814111142";  //รหัสผ่าน WI-FI
#define wifiLed 2            //กำหนดตัวแปร
void setup() {

  pinMode(wifiLed, OUTPUT);    //กำหนดโหมด GPIO ให้เป็นแบบ OUTPUT
  digitalWrite(wifiLed, LOW);  //กำหนดค่าเริ่มต้นให้ Led built-in ดับ

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);


  Serial.begin(9600);
  Serial.println("Starting...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");  //ถ้าเชื่อมต่อ Wi-Fi ไม่สำเร็จให้ Serial print .....................
  }
  Serial.println("WiFi Connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(wifiLed, HIGH);  //สั่งให้ Led built-in ติดหลังจากเชื่อมต่อ Wi-Fi ได้
}

void loop() {
  //run program here

  digitalWrite(relay1, LOW);
  delay(1000);
  digitalWrite(relay1, HIGH);
  delay(1000);

  digitalWrite(relay2, LOW);
  delay(1000);
  digitalWrite(relay2, HIGH);
  delay(1000);

  digitalWrite(relay3, LOW);
  delay(1000);
  digitalWrite(relay3, HIGH);
  delay(1000);

  digitalWrite(relay4, LOW);
  delay(1000);
  digitalWrite(relay4, HIGH);
  delay(1000);

  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  delay(1000);

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
  delay(1000);
}
