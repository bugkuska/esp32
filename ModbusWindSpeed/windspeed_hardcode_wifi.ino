/******************** Libraries ********************/
#include <WiFi.h>               // ไลบรารี WiFi สำหรับใช้งานการเชื่อมต่อเครือข่ายบน ESP32
#include <WiFiClient.h>         // ไลบรารีสำหรับสร้าง client เชื่อมต่อ TCP/IP (เช่น HTTP, MQTT)
#include <BlynkSimpleEsp32.h>   // ไลบรารีสำหรับเชื่อมต่อ Blynk Legacy เวอร์ชัน 0.6.0 บน ESP32
#include <ModbusMaster.h>       // ไลบรารีสำหรับใช้งานโปรโตคอล Modbus RTU แบบ Master (อ่านค่า Slave)

/******************** RS485 / Modbus ********************/
#define RX2_PIN 16              // กำหนดขา RX ของ Serial2 (รับข้อมูลจากโมดูล RS485 → ESP32 GPIO16)
#define TX2_PIN 17              // กำหนดขา TX ของ Serial2 (ส่งข้อมูลจาก ESP32 → โมดูล RS485 GPIO17)

// ปรับตามอุปกรณ์จริง
static const uint8_t  SLAVE_ID   = 1;       // กำหนดหมายเลข Slave ID ของเซ็นเซอร์ (ดูจากคู่มืออุปกรณ์)
static const uint16_t REG_ADDR   = 0x0000;  // ที่อยู่ (Address) ของรีจิสเตอร์ที่เก็บค่าความเร็วลม
static const uint16_t REG_COUNT  = 1;       // จำนวนรีจิสเตอร์ที่ต้องการอ่าน (1 รีจิสเตอร์ ขนาด 16 บิต)
static const uint32_t MODBUS_BAUD= 9600;    // Baudrate ที่ใช้สื่อสาร Modbus (ต้องตรงกับอุปกรณ์ เช่น 4800, 9600, 19200)
static const float    SCALE      = 0.1f;    // ตัวคูณสเกลของค่าที่อ่านมา (raw * 0.1 = ค่าความเร็วลมจริง หน่วย m/s)
static const bool     USE_INPUT_REGS = false; // เลือกชนิดรีจิสเตอร์: true = Input Register, false = Holding Register

ModbusMaster node;                          // สร้างออบเจกต์ node ของ ModbusMaster เพื่อใช้ติดต่อกับอุปกรณ์ Slave

/******************** WiFi & Blynk ********************/
#define WIFI_SSID   ""             // กำหนดชื่อ Wi-Fi (SSID) ที่ ESP32 จะเชื่อมต่อ
#define WIFI_PASS   ""         // กำหนดรหัสผ่านของ Wi-Fi
#define BLYNK_AUTH  ""      // กำหนด Auth Token ที่ได้จากแอป Blynk Legacy

const char*   BLYNK_SERVER = "159.65.5.221"; // กำหนด IP ของ Blynk Server (ในกรณีนี้เป็น IP ของ private server)
const uint16_t BLYNK_PORT  = 8080;           // กำหนด Port ที่ใช้เชื่อมต่อกับ Blynk Server

BlynkTimer timer;                            // สร้างออบเจกต์ timer ของ Blynk สำหรับตั้งเวลาทำงาน (เช่น อ่านค่าและส่งขึ้น Blynk ทุก 2 วินาที)

/******************** Utils ********************/
int readRegister(uint16_t reg) {                   // ฟังก์ชันอ่านค่ารีจิสเตอร์ Modbus ที่ระบุ (reg = address)
  uint8_t result;                                  // เก็บรหัสผลลัพธ์จากการอ่าน (success หรือ error code)

  if (USE_INPUT_REGS)                              // ถ้าเลือกให้ใช้ Input Register
    result = node.readInputRegisters(reg, REG_COUNT);   // อ่านจาก Input Registers (0x04)
  else                                             
    result = node.readHoldingRegisters(reg, REG_COUNT); // อ่านจาก Holding Registers (0x03)

  if (result == node.ku8MBSuccess) {               // ถ้าอ่านสำเร็จ
    return node.getResponseBuffer(0);              // คืนค่าดิบ (raw) 16-bit จาก buffer ตำแหน่ง 0
  } else {                                         // ถ้าอ่านไม่สำเร็จ
    Serial.print("[ERR] Modbus code=0x");          // แสดงข้อความ Error Code
    Serial.println(result, HEX);                   // แสดงรหัส error ในรูปแบบ HEX
    return -1;                                     // คืนค่า -1 เพื่อบอกว่าอ่านไม่สำเร็จ
  }
}

void readAndSendWind() {                           // ฟังก์ชันอ่านค่า wind และส่งขึ้น Blynk
  int raw = readRegister(REG_ADDR);                // อ่านค่า raw จากรีจิสเตอร์ที่กำหนด (REG_ADDR)

  if (raw >= 0) {                                  // ถ้าอ่านได้สำเร็จ (ค่ามากกว่าหรือเท่ากับ 0)
    float wind_ms  = raw * SCALE;                  // แปลงค่าดิบ → m/s โดยคูณด้วย SCALE
    float wind_kmh = wind_ms * 3.6f;               // แปลงจาก m/s → km/h

    // ส่งค่าขึ้น Blynk
    Blynk.virtualWrite(V1, wind_ms);               // V1 = ความเร็วลม หน่วย m/s
    Blynk.virtualWrite(V2, wind_kmh);              // V2 = ความเร็วลม หน่วย km/h

    // Debug แสดงค่าทาง Serial Monitor
    Serial.printf("[OK] raw=%d  %.2f m/s | %.2f km/h\n", raw, wind_ms, wind_kmh);
  } else {
    Serial.println("Failed to read windspeed.");   // ถ้าอ่านไม่สำเร็จ แสดงข้อความแจ้งเตือน
  }
}

/******************** Setup ********************/
void setup() {
  Serial.begin(9600);                            // เริ่ม Serial Monitor ความเร็ว 115200 สำหรับ debug

  // RS485 UART2
  Serial2.begin(MODBUS_BAUD, SERIAL_8N1, RX2_PIN, TX2_PIN); // เริ่ม Serial2 สำหรับคุยกับโมดูล RS485

  // ModbusMaster
  node.begin(SLAVE_ID, Serial2);                   // เริ่มต้นการทำงาน ModbusMaster กำหนด Slave ID + Serial2
  //node.setTimeout(1000);                           // ตั้งค่า timeout = 1000 ms (1 วินาที) ต่อการอ่าน
  node.clearResponseBuffer();                      // ล้าง buffer ที่เก็บ response เดิม

  // WiFi + Blynk
  Blynk.begin(BLYNK_AUTH, WIFI_SSID, WIFI_PASS, BLYNK_SERVER, BLYNK_PORT); 
  // เชื่อมต่อ WiFi และ Blynk Server ด้วย Token, SSID, Password, Server, Port

  // อ่าน & ส่งทุก 2 วินาที
  timer.setInterval(2000, readAndSendWind);        // ตั้ง Timer ให้เรียกฟังก์ชัน readAndSendWind ทุกๆ 2000 ms (2 วินาที)
}

/******************** Loop ********************/
void loop() {
  Blynk.run();                                     // ฟังก์ชันหลักของ Blynk ต้องเรียกตลอดเวลา เพื่อ maintain connection
  timer.run();                                     // ตรวจสอบและเรียกใช้ task จาก BlynkTimer (เช่น อ่านค่าเป็นรอบ)
}
