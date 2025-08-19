/******************** Libraries ********************/
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h>          // tzapu/WiFiManager
#include <Preferences.h>          // เก็บ Blynk token
#include <BlynkSimpleEsp32.h>     // Blynk Legacy 0.6.x
#include <ModbusMaster.h>

/******************** RS485 / Modbus ********************/
#define RX2_PIN 16
#define TX2_PIN 17

// ปรับตามอุปกรณ์จริง
static const uint8_t  SLAVE_ID = 1;     // แก้เป็น ID ของมิเตอร์/เซ็นเซอร์คุณ
static const uint16_t REG_ADDR = 0x0000;// แก้เป็นรีจิสเตอร์ที่เก็บความเร็วลม
static const uint16_t REG_COUNT = 1;    // ค่าความเร็วลม 1 รีจิสเตอร์ (U16)
static const uint32_t MODBUS_BAUD = 9600;
static const float    SCALE = 0.1f;     // ตัวคูณแปลงหน่วย (เช่น raw=25 -> 2.5 m/s)

// หากอุปกรณ์ต้องสลับ DE/RE ก่อน/หลังคุย ให้ใช้ callback
void preTransmission();
void postTransmission();

ModbusMaster node;

/******************** Blynk Legacy Server ********************/
const char* BLYNK_SERVER = "159.65.5.221";
const uint16_t BLYNK_PORT = 8080;

// จะโหลด/บันทึกใน NVS
Preferences prefs;
String blynkToken = "";

/******************** WiFiManager custom param ********************/
WiFiManager wm;
WiFiManagerParameter param_blynk_token("blynk", "Blynk Auth Token", "", 40);

/******************** Timers ********************/
BlynkTimer timer;

/******************** Helpers ********************/
void saveTokenToNVS(const String& token) {
  prefs.begin("blynk", false);
  prefs.putString("token", token);
  prefs.end();
}
String readTokenFromNVS() {
  prefs.begin("blynk", true);
  String t = prefs.getString("token", "");
  prefs.end();
  return t;
}

/******************** Read & Send ********************/
void readAndSendWind() {
  // อ่าน Modbus
  uint8_t result = node.readHoldingRegisters(REG_ADDR, REG_COUNT);
  if (result == node.ku8MBSuccess) {
    uint16_t raw = node.getResponseBuffer(0);
    float wind = raw * SCALE;   // หน่วยตามที่คุณตั้ง (เช่น m/s)

    // ส่งขึ้น Blynk: V1
    Blynk.virtualWrite(V1, wind);

    // Debug
    Serial.print("[OK] Wind raw=");
    Serial.print(raw);
    Serial.print("  value=");
    Serial.println(wind);

  } else {
    Serial.print("[ERR] Modbus code=");
    Serial.println(result);
  }
}

/******************** Setup ********************/
void setup() {
  Serial.begin(9600);

  // Serial2 for RS485
  Serial2.begin(MODBUS_BAUD, SERIAL_8N1, RX2_PIN, TX2_PIN);

  // ModbusMaster
  node.begin(SLAVE_ID, Serial2);

  // ----- WiFiManager -----
  // โหลด token เดิม ถ้ามี
  blynkToken = readTokenFromNVS();

  // เพิ่มช่องกรอก Token ลงหน้า config
  if (blynkToken.length() > 0) {
    param_blynk_token.setValue(blynkToken.c_str(), blynkToken.length());
  }
  wm.addParameter(&param_blynk_token);

  // ชื่อ AP ตอนตั้งค่า
  wm.setConfigPortalBlocking(true);
  wm.setMinimumSignalQuality(0);
  wm.setBreakAfterConfig(true); // ออกจาก loop.startConfigPortal เมื่อกด Save & Reboot

  // ถ้ายังไม่เคยตั้งค่า จะเข้าโหมด AP อัตโนมัติ
  bool ok = wm.autoConnect("WindSpeed-Setup");
  if (!ok) {
    Serial.println("Failed to connect. Rebooting...");
    delay(3000);
    ESP.restart();
  }

  // ดึง token จากหน้า config (ล่าสุด)
  String newToken = String(param_blynk_token.getValue());
  newToken.trim();
  if (newToken.length() > 0 && newToken != blynkToken) {
    blynkToken = newToken;
    saveTokenToNVS(blynkToken);
  }

  // ----- Blynk -----
  // ใช้ server & port แบบกำหนดเอง
  Blynk.config(blynkToken.c_str(), BLYNK_SERVER, BLYNK_PORT);

  // ถ้าต้องการรอจนเชื่อมได้ค่อยไปต่อ (optional)
  if (!Blynk.connect(8000)) { // รอ 8 วินาที
    Serial.println("Blynk connect timeout (will keep trying in loop).");
  }

  // อ่าน & ส่งทุก 2 วินาที
  timer.setInterval(2000, readAndSendWind);
}

/******************** Loop ********************/
void loop() {
  Blynk.run();
  timer.run();
}
