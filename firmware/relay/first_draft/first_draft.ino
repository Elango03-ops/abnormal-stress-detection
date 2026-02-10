#include "HX711.h"

#define HX_DT 19
#define HX_SCK 18

#define RELAY_PIN 23
#define GREEN_LED 32
#define RED_LED 33

HX711 scale;

// ---- parameters ----
const int BASELINE_SAMPLES = 50;
const int AVG_SAMPLES = 10;
const long ALERT_DELTA = 25000;

long baseline = 0;
bool alertState = false;

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

void setup()
{
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  scale.begin(HX_DT, HX_SCK);

  Serial.println("Stabilizing HX711...");
  delay(2000);

  Serial.println("Calculating baseline...");
  baseline = readAverage(BASELINE_SAMPLES);

  Serial.print("Baseline value: ");
  Serial.println(baseline);
}

void loop()
{
  long avgValue = readAverage(AVG_SAMPLES);
  long delta = avgValue - baseline;

  if (!alertState && delta >= ALERT_DELTA)
  {
    alertState = true;
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    Serial.println("ALERT TRIGGERED");
  }

  if (alertState && delta < (ALERT_DELTA * 0.6))
  {
    alertState = false;
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    Serial.println("BACK TO NORMAL");
  }

  Serial.print("AVG: ");
  Serial.print(avgValue);
  Serial.print("  DELTA: ");
  Serial.print(delta);
  Serial.print("  STATE: ");
  Serial.println(alertState ? "ALERT" : "NORMAL");

  delay(300);
}
