#include <ModbusMaster.h>

#define RX1 16  // RO, RX
#define TX1 17  // DI, TX

ModbusMaster soilSensor;  // Modbus ID 1

// ตั้งเวลาการอ่าน (ทุก 5 วินาที)
unsigned long lastReadTime = 0;
const unsigned long interval = 5000;

// ฟังก์ชันสำหรับอ่านข้อมูลจากเซ็นเซอร์
void readSoilSensorData() {
  uint8_t result = soilSensor.readHoldingRegisters(0x0000, 7);  // อ่าน 7 ค่า

  if (result == soilSensor.ku8MBSuccess) {
    float humidity    = soilSensor.getResponseBuffer(0) * 0.1f;
    float temperature = (int16_t)soilSensor.getResponseBuffer(1) * 0.1f;
    uint16_t ec       = soilSensor.getResponseBuffer(2);
    float ph          = soilSensor.getResponseBuffer(3) * 0.1f;
    uint16_t nitrogen = soilSensor.getResponseBuffer(4);
    uint16_t phosphorus = soilSensor.getResponseBuffer(5);
    uint16_t potassium  = soilSensor.getResponseBuffer(6);

    Serial.println("----- Soil Sensor Readings -----");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %RH");
    Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
    Serial.print("EC: "); Serial.print(ec); Serial.println(" uS/cm");
    Serial.print("pH: "); Serial.println(ph);
    Serial.print("Nitrogen (N): "); Serial.print(nitrogen); Serial.println(" mg/kg");
    Serial.print("Phosphorus (P): "); Serial.print(phosphorus); Serial.println(" mg/kg");
    Serial.print("Potassium (K): "); Serial.print(potassium); Serial.println(" mg/kg");
    Serial.println("--------------------------------\n");

  } else {
    Serial.print("Modbus Read Error. Code: ");
    Serial.println(result);
  }
}

void setup() {
  Serial.begin(9600);
  Serial2.begin(4800, SERIAL_8N1, RX1, TX1);
  soilSensor.begin(1, Serial2);  // Modbus ID = 1
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastReadTime >= interval) {
    lastReadTime = currentMillis;
    readSoilSensorData();  // เรียกฟังก์ชันอ่านข้อมูล
  }

  // สามารถทำงานอื่น ๆ ได้ใน loop นี้
}
