//#define INPUT_1 35
//#define INPUT_2 34
#define INPUT_3 39
//#define INPUT_4 36

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);     
}
void loop() {
  float ldr_percentage3;
  int sensor_analog3;
  sensor_analog3 = analogRead(INPUT_3);
  Serial.print("Law LDR data 3:");
  Serial.println(sensor_analog3);
  ldr_percentage3 = ( 100 - ( (sensor_analog3/4095.00) * 100 ) );
  
  Serial.print("LDR Percentage 3 = ");
  Serial.print(ldr_percentage3);
  Serial.print("%\n\n");
  delay(1000);  
}
