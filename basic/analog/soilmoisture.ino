#define INPUT_1 35
//#define INPUT_2 34
//#define INPUT_3 36
//#define INPUT_4 39
#define relaypin 25

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(relaypin, OUTPUT);
  digitalWrite(relaypin, LOW);
}
void loop() {
  float moisture_percentage1;
  int sensor_analog1;
  sensor_analog1 = analogRead(INPUT_1);
  Serial.print("Law Soil data 1:");
  Serial.println(sensor_analog1);
  moisture_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));

  Serial.print("Moisture Percentage 1= ");
  Serial.print(moisture_percentage1);
  Serial.print("%\n\n");
  delay(1000);

  if (moisture_percentage1 < 50) {
    digitalWrite(relaypin, HIGH);
  } else {
    digitalWrite(relaypin, LOW);
  }
}
