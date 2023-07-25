//#define INPUT_1 35
#define INPUT_2 34
//#define INPUT_3 36
//#define INPUT_4 39

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);     
}
void loop() {
  int sensor_analog2;
  sensor_analog2 = analogRead(INPUT_2);
  Serial.print("Methane gas ADC : ");
  Serial.println(sensor_analog2);
  delay(1000);
}
