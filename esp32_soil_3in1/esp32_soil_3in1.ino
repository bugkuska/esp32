#include <Wire.h> 
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ModbusMaster.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <TimeLib.h>        // (ยังใช้ได้ หากอยากแสดงเวลา V7/V8)
#include <WidgetRTC.h>

WidgetRTC rtc;

// ===== Blynk Auth/Server =====
const char auth[] = "";   // blynk legacy token
const char blynk_host[] = ""; // blynk legacy server ip or host
const uint16_t blynk_port = 8080; // blynk legacy port (default 8080)

// ===== UART2 (RS485) =====
#define RXD 16
#define TXD 17

// ===== Extend Relays (Active-Low) =====
#define EXT_PUMP_PIN   18   // ปั๊มน้ำ (Active-Low)
#define EXT_PILOT_PIN  19   // Pilot Lamp: Blynk Connected (Active-Low)

// ===== Onboard Relays (Active-High) =====
#define VALVE1_PIN 26
#define VALVE2_PIN 25
#define VALVE3_PIN 33
#define VALVE4_PIN 32

// ===== LED Built-in =====
#define LED_PIN 2    // ติดเมื่อ WiFi connected

// ===== Blynk Virtual Pins =====
#define VPUMP             V1   // ปุ่มปั๊ม (Manual only)
#define VVALVE1           V2   // ปุ่ม Valve1 (Manual only)
#define VVALVE2           V3   // ปุ่ม Valve2 (Manual only)
#define VVALVE3           V13  // ปุ่ม Valve3 (Manual only)
#define VVALVE4           V14  // ปุ่ม Valve4 (Manual only)
#define SoilMoisture_VPin V4   // แสดงค่าความชื้น
#define AutoManual_VPin   V5   // Eventor Toggle: 1=Auto, 0=Manual
#define SoilThreshold_VPin V6  // Slider เกณฑ์
#define Time_VPin          V7  // เวลา (option)
#define Date_VPin          V8  // วันที่ (option)

// >>> ADD: Virtual pins สำหรับ 3-in-1
#define GZWS_RH_VPIN      V9   // ความชื้น (%RH)
#define GZWS_TC_VPIN      V10  // อุณหภูมิ (°C)
#define GZWS_LX_VPIN      V11  // แสง (Lux)

// ===== Sequencer config (ดีเลย์ 2 วิ) =====
const unsigned long OPEN_LEAD_MS = 2000;   // เปิด Valve1 -> รอ 2 วิ -> เปิดปั๊ม
const unsigned long CLOSE_LAG_MS = 2000;   // ปิดปั๊ม -> รอ 2 วิ -> ปิด Valve1

// ===== Globals =====
ModbusMaster node1;   // Modbus sensor (ID=1)
// >>> ADD: อินสแตนซ์ Modbus สำหรับ 3-in-1 (Slave ID=2)
ModbusMaster nodeGZWS;  // ID2  (เช่น PR-3002-GZWS-N01, 3-in-1)

WiFiManager wm;
BlynkTimer timer;
Preferences preferences;

bool isAutoMode = true;       // เริ่มต้น Auto ตามสเปค
float soilThreshold = 50.0;   // %
float soilMoisture = 0.0;     // %
bool pumpOn=false, pilotOn=false;
bool v1On=false, v2On=false, v3On=false, v4On=false;

// ---- Auto sequencer (non-blocking) ----
enum SeqState { SEQ_IDLE, SEQ_WAIT_PUMP, SEQ_ON, SEQ_WAIT_V1_CLOSE };
SeqState seqState = SEQ_IDLE;
unsigned long seqTs = 0;

void setWifiLed(bool on) {
  // ถ้า LED ของบอร์ดคุณเป็น Active-Low ให้สลับ HIGH/LOW ที่นี่
  digitalWrite(LED_PIN, on ? HIGH : LOW);
}
void setPilot(bool on) {       // Active-Low
  pilotOn = on;
  digitalWrite(EXT_PILOT_PIN, on ? LOW : HIGH);
}
void setPump(bool on) {        // Active-Low
  pumpOn = on;
  digitalWrite(EXT_PUMP_PIN, on ? LOW : HIGH);
}
void setValve1(bool on){ v1On = on; digitalWrite(VALVE1_PIN, on ? HIGH : LOW); }
void setValve2(bool on){ v2On = on; digitalWrite(VALVE2_PIN, on ? HIGH : LOW); }
void setValve3(bool on){ v3On = on; digitalWrite(VALVE3_PIN, on ? HIGH : LOW); }
void setValve4(bool on){ v4On = on; digitalWrite(VALVE4_PIN, on ? HIGH : LOW); }

void reflectButtons(){
  Blynk.virtualWrite(VPUMP,   pumpOn);
  Blynk.virtualWrite(VVALVE1, v1On);
  Blynk.virtualWrite(VVALVE2, v2On);
  Blynk.virtualWrite(VVALVE3, v3On);
  Blynk.virtualWrite(VVALVE4, v4On);
}

void checkConnections(){
  // Wi-Fi
  bool wifiOK = (WiFi.status() == WL_CONNECTED);
  if (!wifiOK){
    setWifiLed(false);
    Serial.println("WiFi down → reconnect");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin();

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 6000){
      delay(250); Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED){
      Serial.println("\nOpen ConfigPortal...");
      wm.setEnableConfigPortal(true);
      wm.setConfigPortalTimeout(180);
      wm.startConfigPortal("SMFThailand-Setup", "0814111142");
    }else{
      Serial.println("\nWiFi reconnected.");
    }
  }
  setWifiLed(WiFi.status() == WL_CONNECTED);

  // Blynk
  if (!Blynk.connected()){
    Blynk.connect();
    setPilot(false);  // หลุด → ดับ
  }else{
    setPilot(true);   // ต่อได้ → ติด
  }
}

// ===== Sequencer helpers =====
// เรียกเมื่อ “ต้องการเปิดน้ำจาก Auto”
void autoStart(){
  // เปิด Valve1 ก่อน แล้วรอ 2 วิค่อยเปิดปั๊ม
  if (!v1On) setValve1(true);
  Blynk.virtualWrite(VVALVE1, v1On);
  seqTs = millis();
  seqState = SEQ_WAIT_PUMP;
}

// เรียกเมื่อ “ต้องการปิดน้ำจาก Auto”
void autoStop(){
  // ปิดปั๊มก่อน แล้วรอ 2 วิค่อยปิด Valve1
  if (pumpOn) setPump(false);
  Blynk.virtualWrite(VPUMP, pumpOn);
  seqTs = millis();
  seqState = SEQ_WAIT_V1_CLOSE;
}

void processSequencer(){
  if (!isAutoMode){
    seqState = SEQ_IDLE;
    return;
  }
  switch (seqState){
    case SEQ_WAIT_PUMP:
      if (millis() - seqTs >= OPEN_LEAD_MS){
        if (!pumpOn) setPump(true);
        Blynk.virtualWrite(VPUMP, pumpOn);
        seqState = SEQ_ON;
      }
      break;
    case SEQ_ON:
      // รันอยู่ ไม่ต้องทำอะไรที่นี่ (การหยุดจะเริ่มจาก autoStop())
      break;
    case SEQ_WAIT_V1_CLOSE:
      if (millis() - seqTs >= CLOSE_LAG_MS){
        if (v1On) setValve1(false);
        Blynk.virtualWrite(VVALVE1, v1On);
        seqState = SEQ_IDLE;
      }
      break;
    case SEQ_IDLE:
    default:
      break;
  }
}

// ---------- Modbus Soil Moisture (ID1) ----------
void readSoilMoisture(){
  Serial.println("Modbus: read soil moisture...");
  uint8_t res = node1.readHoldingRegisters(0x0000, 3);  // ปรับเรจิสเตอร์/จำนวนให้ตรงรุ่น
  if (res == node1.ku8MBSuccess){
    float v = node1.getResponseBuffer(2) / 10.0f;       // สมมติสเกล 0.1
    if (v >= 0.0f && v <= 100.0f){
      soilMoisture = v;
      Blynk.virtualWrite(SoilMoisture_VPin, soilMoisture);
      preferences.putFloat("soil_moisture", soilMoisture);
      Serial.printf("Soil=%.1f%% / Th=%.1f%% | mode=%s\n", soilMoisture, soilThreshold, isAutoMode?"AUTO":"MANUAL");

      // ✅ Auto คุมเฉพาะ "Valve1 + Pump" ด้วยลำดับหน่วง 2 วิ
      if (isAutoMode){
        if (soilMoisture < soilThreshold){
          // ต้องการเปิด: V1 -> delay -> Pump
          if (seqState == SEQ_IDLE || seqState == SEQ_WAIT_V1_CLOSE){
            autoStart();
          }
        }else{
          // ต้องการปิด: Pump -> delay -> V1
          if (seqState == SEQ_ON || seqState == SEQ_WAIT_PUMP){
            autoStop();
          }
        }
      }
    }
  }else{
    Serial.println("Modbus read failed!");
  }
}

// >>> ADD: ---------- 3-in-1 Reader (ID2) ----------
void readGZWS(){ // ตัวอย่าง: PR-3002-GZWS-N01 แบบ 3-in-1 (RH/°C/Lux)
  // ใช้ nodeGZWS (ID=2) บน Serial2 เดียวกัน
  uint8_t rc = nodeGZWS.readHoldingRegisters(0x0000, 3);
  if (rc == nodeGZWS.ku8MBSuccess) {
    float    rh = nodeGZWS.getResponseBuffer(0) * 0.1f; // %RH
    float   t_c = nodeGZWS.getResponseBuffer(1) * 0.1f; // °C
    uint16_t lx = nodeGZWS.getResponseBuffer(2);        // Lux (integer)

    // ส่งขึ้น Blynk ตามที่กำหนด
    Blynk.virtualWrite(GZWS_RH_VPIN,  rh);
    Blynk.virtualWrite(GZWS_TC_VPIN,  t_c);
    Blynk.virtualWrite(GZWS_LX_VPIN,  lx);

    // Log ลง Serial
    Serial.print("ID2 3-in-1  Humid: ");
    Serial.print(rh, 1);
    Serial.print(" %RH | Temp: ");
    Serial.print(t_c, 1);
    Serial.print(" °C | Lux: ");
    Serial.println(lx);
  } else {
    Serial.print("ID2 3-in-1 read failed, code=");
    Serial.println(rc);
  }
}

// ---------- Blynk Events ----------
BLYNK_CONNECTED(){
  Serial.println("Blynk connected → sync");
  setPilot(true);
  Blynk.syncVirtual(AutoManual_VPin, SoilThreshold_VPin,
                    VPUMP, VVALVE1, VVALVE2, VVALVE3, VVALVE4,
                    SoilMoisture_VPin, Time_VPin, Date_VPin
                    // >>> ADD: ถ้าต้องการ sync ก็เพิ่มได้ แต่ไม่จำเป็นสำหรับ display-only
                    /* , GZWS_RH_VPIN, GZWS_TC_VPIN, GZWS_LX_VPIN */ );
  Blynk.virtualWrite(AutoManual_VPin, isAutoMode);
  reflectButtons();

  rtc.begin();
  setSyncInterval(600);
}

// Eventor จะเขียนค่านี้ (1=Auto, 0=Manual)
BLYNK_WRITE(AutoManual_VPin){
  isAutoMode = param.asInt();
  preferences.putBool("auto_mode", isAutoMode);
  Serial.printf("Mode by Eventor: %s\n", isAutoMode ? "AUTO" : "MANUAL");

  if (!isAutoMode){
    // Manual: ปิดเฉพาะ ปั๊ม + Valve1 ทันที และรีเซ็ต sequencer
    setPump(false);
    setValve1(false);
    seqState = SEQ_IDLE;
    Blynk.virtualWrite(VPUMP,   pumpOn);
    Blynk.virtualWrite(VVALVE1, v1On);
    reflectButtons();  // อัปเดตปุ่มทั้งหมดตามสถานะจริง
  }
  Blynk.virtualWrite(AutoManual_VPin, isAutoMode);
}

BLYNK_WRITE(SoilThreshold_VPin){
  soilThreshold = param.asFloat();
  preferences.putFloat("soil_threshold", soilThreshold);
  Serial.printf("Threshold=%.1f%%\n", soilThreshold);
  // ไม่บังคับสั่งงานทันที ปล่อยให้รอบอ่านเซ็นเซอร์ถัดไปตัดสิน
}

// ---------- Manual control ----------
// ปั๊ม + Valve1: ใช้ได้เฉพาะตอน Manual (Auto จะเมินปุ่ม)
BLYNK_WRITE(VPUMP)   { if (!isAutoMode){ setPump(param.asInt());   } Blynk.virtualWrite(VPUMP, pumpOn); }
BLYNK_WRITE(VVALVE1) { if (!isAutoMode){ setValve1(param.asInt()); } Blynk.virtualWrite(VVALVE1, v1On); }

// Valve2/3/4: คุมได้ตลอดเวลา (Auto/Manual) — Auto ไม่แตะ Valves 2–4 อยู่แล้ว
BLYNK_WRITE(VVALVE2) { setValve2(param.asInt());  Blynk.virtualWrite(VVALVE2, v2On); }
BLYNK_WRITE(VVALVE3) { setValve3(param.asInt());  Blynk.virtualWrite(VVALVE3, v3On); }
BLYNK_WRITE(VVALVE4) { setValve4(param.asInt());  Blynk.virtualWrite(VVALVE4, v4On); }

// ---------- Time to Blynk (optional) ----------
void sendDateTimeToBlynk(){
  if (Blynk.connected() && year() > 1970){
    char tbuf[10], dbuf[15];
    sprintf(tbuf, "%02d:%02d:%02d", hour(), minute(), second());
    sprintf(dbuf, "%02d/%02d/%04d", day(), month(), year());
    Blynk.virtualWrite(Time_VPin, tbuf);
    Blynk.virtualWrite(Date_VPin, dbuf);
  }
}

// ---------- Setup / Loop ----------
void setup(){
  Serial.begin(9600);

  // Outputs
  pinMode(EXT_PUMP_PIN, OUTPUT);
  pinMode(EXT_PILOT_PIN, OUTPUT);
  pinMode(VALVE1_PIN, OUTPUT);
  pinMode(VALVE2_PIN, OUTPUT);
  pinMode(VALVE3_PIN, OUTPUT);
  pinMode(VALVE4_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // เริ่มต้นปิดทั้งหมด
  setPump(false); setPilot(false);
  setValve1(false); setValve2(false); setValve3(false); setValve4(false);
  setWifiLed(false);

  // Preferences
  preferences.begin("sensor_data", false);
  isAutoMode    = preferences.getBool("auto_mode", true);
  soilThreshold = preferences.getFloat("soil_threshold", 50.0);
  soilMoisture  = preferences.getFloat("soil_moisture", 0.0);

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 8000){ delay(250); Serial.print("."); }
  if (WiFi.status() != WL_CONNECTED){
    wm.setCleanConnect(true);
    wm.setEnableConfigPortal(true);
    wm.setConfigPortalTimeout(180);
    wm.startConfigPortal("ESP32Dev-Soil01");
  }
  Serial.println("\nWiFi ready.");
  Serial.print("IP: "); Serial.println(WiFi.localIP());
  setWifiLed(WiFi.status() == WL_CONNECTED);

  // Blynk (non-blocking)
  Blynk.config(auth, blynk_host, blynk_port);
  Blynk.connect();

  // Modbus
  Serial2.begin(9600, SERIAL_8N1, RXD, TXD);
  node1.begin(1, Serial2);     // ID1: Soil Moisture
  nodeGZWS.begin(2, Serial2);  // >>> ADD: ID2: 3-in-1

  // Timers
  timer.setInterval(10000L, checkConnections);
  timer.setInterval(15000L, readSoilMoisture); // ID1 ทุก 15s
  timer.setInterval(17000L, readGZWS);         // >>> ADD: ID2 ทุก 17s (สลับเฟส)
  timer.setInterval(10000L, sendDateTimeToBlynk);
  timer.setInterval(100L,   processSequencer);  // 🔄 เดินลำดับหน่วง 2 วิ (10Hz)
}

void loop(){
  if (Blynk.connected()) Blynk.run();
  timer.run();
}
