#include <Wire.h>
#include "HX711.h"

// ===== HX711 pins (ESP32) =====
#define HX711_DOUT 19
#define HX711_SCK  18

// ===== MPU6050 =====
#define MPU_ADDR 0x68

HX711 scale;

// Baselines
long loadBaseline = 0;
long vibBaseline = 0;

// Thresholds (tune later)
#define LOAD_THRESHOLD 3000
#define VIB_THRESHOLD_FACTOR 1.3

void setup() {
  Serial.begin(115200);
  delay(2000);

  // HX711
  scale.begin(HX711_DOUT, HX711_SCK);

  // I2C for MPU6050
  Wire.begin(21, 22);

  // Wake MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  Serial.println("Fusion v1 starting...");
  delay(2000);

  // Learn baselines
  loadBaseline = scale.read();
  vibBaseline = readVibration();

  Serial.print("Load baseline: ");
  Serial.println(loadBaseline);
  Serial.print("Vibration baseline: ");
  Serial.println(vibBaseline);
}

void loop() {
  long loadValue = scale.read();
  long loadDiff = labs(loadValue - loadBaseline);

  long vibValue = readVibration();

  Serial.print("Load: ");
  Serial.print(loadValue);
  Serial.print(" | Diff: ");
  Serial.print(loadDiff);
  Serial.print(" | Vib: ");
  Serial.print(vibValue);

  if (loadDiff > LOAD_THRESHOLD &&
      vibValue < (long)(vibBaseline * VIB_THRESHOLD_FACTOR)) {
    Serial.println("  >>> FAULT (real stress)");
  } else {
    Serial.println("  NORMAL");
  }

  delay(500);
}

// ----- Read vibration magnitude (squared) -----
long readVibration() {
  int16_t ax, ay, az;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();

  return (long)ax * ax + (long)ay * ay + (long)az * az;
}
