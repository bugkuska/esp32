#include <WiFi.h>

char ssid[] = ""; //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = ""; //รหัสผ่าน WI-FI
 #define wifiLed 2  //กำหนดตัวแปร
void setup()
{
 
 pinMode(wifiLed, OUTPUT);  //กำหนดโหมด GPIO ให้เป็นแบบ OUTPUT
 digitalWrite(wifiLed, LOW); //กำหนดค่าเริ่มต้นให้ Led built-in ดับ
 
   Serial.begin(9600); 
   Serial.println("Starting...");
   WiFi.begin(ssid,pass);
   while (WiFi.status() != WL_CONNECTED)
   {
    delay(250);
    Serial.print(".");  //ถ้าเชื่อมต่อ Wi-Fi ไม่สำเร็จให้ Serial print .....................
   }
  Serial.println("WiFi Connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
 
 digitalWrite(wifiLed, HIGH);  //สั่งให้ Led built-in ติดหลังจากเชื่อมต่อ Wi-Fi ได้
 }

void loop()
{
 //run program here 
}
