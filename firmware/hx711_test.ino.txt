#include "HX711.h"

// ESP32 pins
#define HX711_DOUT 19
#define HX711_SCK  18

HX711 scale;

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("HX711 raw test starting...");

  scale.begin(HX711_DOUT, HX711_SCK);

  Serial.println("HX711 ready");
}

void loop() {
  if (scale.is_ready()) {
    long value = scale.read();
    Serial.print("HX711 value: ");
    Serial.println(value);
  } else {
    Serial.println("HX711 not ready");
  }

  delay(500);
}
