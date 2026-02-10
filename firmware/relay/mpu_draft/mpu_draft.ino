#include "HX711.h"
#include <Wire.h>
#include <math.h>

// ---------- HX711 ----------
#define HX_DT 19
#define HX_SCK 18

// ---------- OUTPUTS ----------
#define RELAY_PIN 23
#define GREEN_LED 32
#define RED_LED 33

// ---------- MPU ----------
#define MPU_ADDR 0x68

HX711 scale;

// ---------- HX711 PARAMETERS ----------
const int BASELINE_SAMPLES = 50;
const int AVG_SAMPLES = 10;
const long ALERT_DELTA = 25000;

// ---------- MPU PARAMETERS ----------
const float SHOCK_THRESHOLD = 1.5; // g change

long baseline = 0;
bool alertState = false;

// ---------- HX711 ----------
long readAverage(int samples)
{
  long sum = 0;
  for (int i = 0; i < samples; i++)
  {
    sum += scale.read();
    delay(10);
  }
  return sum / samples;
}

// ---------- MPU ----------
float readAccelMagnitude()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // ACCEL_XOUT_H
  Wire.endTransmission(false);

  Wire.requestFrom(uint8_t(MPU_ADDR), uint8_t(6), true);

  int16_t ax = (Wire.read() << 8) | Wire.read();
  int16_t ay = (Wire.read() << 8) | Wire.read();
  int16_t az = (Wire.read() << 8) | Wire.read();

  float axg = ax / 16384.0;
  float ayg = ay / 16384.0;
  float azg = az / 16384.0;

  return sqrt(axg * axg + ayg * ayg + azg * azg);
}

void setup()
{
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  // HX711
  scale.begin(HX_DT, HX_SCK);
  delay(2000);

  baseline = readAverage(BASELINE_SAMPLES);
  Serial.print("HX711 baseline: ");
  Serial.println(baseline);

  // MPU init
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1
  Wire.write(0);    // wake up MPU
  Wire.endTransmission(true);

  Serial.println("MPU initialized");
}

void loop()
{
  // ----- HX711 -----
  long avgValue = readAverage(AVG_SAMPLES);
  long delta = avgValue - baseline;
  bool hxAlert = delta >= ALERT_DELTA;

  // ----- MPU -----
  static float lastMag = 1.0;
  float mag = readAccelMagnitude();
  float shock = fabs(mag - lastMag);
  lastMag = mag;

  bool mpuAlert = shock > SHOCK_THRESHOLD;

  // ----- DECISION -----
  if (!alertState && (hxAlert || mpuAlert))
  {
    alertState = true;
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    Serial.println("ALERT TRIGGERED");
  }

  if (alertState && delta < (ALERT_DELTA * 0.6) && !mpuAlert)
  {
    alertState = false;
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    Serial.println("BACK TO NORMAL");
  }

  // ----- DEBUG -----
  Serial.print("HX Δ: ");
  Serial.print(delta);
  Serial.print(" | SHOCK: ");
  Serial.print(shock, 2);
  Serial.print(" | STATE: ");
  Serial.println(alertState ? "ALERT" : "NORMAL");

  delay(200);
}
