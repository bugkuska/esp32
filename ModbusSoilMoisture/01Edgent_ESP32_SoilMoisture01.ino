// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_WROVER_BOARD
#include "BlynkEdgent.h"

//=====Setup Function=====//
void setup()
{
  Serial.begin(9600);
  delay(100);

  BlynkEdgent.begin();
}
//=====Setup Function=====//

//=====Loop Function=====//
void loop() {
  BlynkEdgent.run();
}
//=====Loop Function=====//
