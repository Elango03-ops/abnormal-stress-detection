#include <Wire.h>

#define MPU_ADDR 0x68

void setup() {
  Serial.begin(115200);
  delay(2000);

  Wire.begin(21, 22); // SDA, SCL for ESP32

  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);      // PWR_MGMT_1
  Wire.write(0x00);      // wake up
  Wire.endTransmission();

  Serial.println("MPU6050 test started");
}

void loop() {
  int16_t ax, ay, az;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();

  long magnitude = (long)ax * ax + (long)ay * ay + (long)az * az;

  Serial.print("AX: "); Serial.print(ax);
  Serial.print(" AY: "); Serial.print(ay);
  Serial.print(" AZ: "); Serial.print(az);
  Serial.print(" | MAG: "); Serial.println(magnitude);

  delay(200);
}
