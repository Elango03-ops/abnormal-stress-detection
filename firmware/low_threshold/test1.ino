#include "HX711.h"
#include <Wire.h>
#include <math.h>

// ---------- PINS ----------
#define HX_DT 19
#define HX_SCK 18
#define RELAY_PIN 23
#define GREEN_LED 32
#define RED_LED 33
#define MPU_ADDR 0x68

HX711 scale;

// ---------- HX711 PARAMETERS ----------
const int BASELINE_SAMPLES = 50;
const int AVG_SAMPLES = 10;
const long ALERT_DELTA = 25000;

// ---------- MPU PARAMETERS ----------
const float SHOCK_THRESHOLD = 0.4; // lower = more sensitive, raise if false triggers

long baseline = 0;
bool alertState = false;
static float lastMag = 1.0;

// ---------- FUNCTIONS ----------
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

float readAccelMagnitude()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
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

// ---------- SETUP ----------
void setup()
{
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  // MPU init FIRST before any Wire usage
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0); // wake up
  Wire.endTransmission(true);
  Serial.println("MPU initialized");

  // HX711 init
  scale.begin(HX_DT, HX_SCK);
  delay(2000);
  baseline = readAverage(BASELINE_SAMPLES);
  Serial.print("HX711 baseline: ");
  Serial.println(baseline);

  Serial.println("System ready.");
}

// ---------- LOOP ----------
void loop()
{
  // ----- MPU FIRST (fast, before slow HX711 averaging) -----
  float mag = readAccelMagnitude();
  float shock = fabs(mag - lastMag);
  lastMag = mag;
  bool mpuAlert = shock > SHOCK_THRESHOLD;

  // ----- HX711 (slow) -----
  long avgValue = readAverage(AVG_SAMPLES);
  long delta = avgValue - baseline;
  bool hxAlert = delta >= ALERT_DELTA;

  // ----- ALERT TRIGGER -----
  if (!alertState && (hxAlert || mpuAlert))
  {
    alertState = true;
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    Serial.print(">>> ALERT TRIGGERED | Cause: ");
    Serial.println(mpuAlert ? "VIBRATION" : "LOAD");
  }

  // ----- RETURN TO NORMAL -----
  if (alertState && delta < (ALERT_DELTA * 0.6) && !mpuAlert)
  {
    alertState = false;
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    Serial.println(">>> BACK TO NORMAL");
  }

  // ----- DEBUG -----
  Serial.print("HX Δ: ");
  Serial.print(delta);
  Serial.print(" | SHOCK: ");
  Serial.print(shock, 3);
  Serial.print(" | STATE: ");
  Serial.println(alertState ? "ALERT" : "NORMAL");

  delay(100); // reduced from 200 for faster MPU response
}