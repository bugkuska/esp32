#include<Wire.h>
#include<LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

//dht11
#include <DHT.h>                  //https://github.com/adafruit/DHT-sensor-library
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();  
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  Serial.print("Temperature:");         
  Serial.println(t);
  Serial.print("Humidity:");
  Serial.println(h);
  delay(1000);

  //LCD
      lcd.begin();
      lcd.backlight();                                   
      lcd.setCursor(1, 0); 
      lcd.print("TEMP =  "); 
      lcd.print(t);
      lcd.print("  C ");
      
      lcd.setCursor(1, 1);
      lcd.print("HUMI =  "); 
      lcd.print(h);
      lcd.print("  % ");
}
