# Abnormal Stress Detection (Draft-1)

This repository contains a **Draft-1 prototype** for detecting abnormal mechanical stress using sensor fusion on an ESP32.

The goal is to distinguish **real mechanical stress** from **environmental disturbance** (vibration, movement).

---

## Problem Statement
Mechanical systems often fail after prolonged or abnormal stress.
Simple threshold-based sensing gives false alarms due to vibration, wind, or movement.

This draft explores whether **sensor fusion** can improve reliability.

---

## Approach
Two sensors are used:

### 1. Load Cell + HX711
- Measures **relative mechanical strain**
- Very sensitive to force
- Cannot distinguish vibration vs real load

### 2. MPU6050 (Accelerometer)
- Measures **vibration and sudden motion**
- Helps identify environmental disturbances

### Fusion Logic (Draft-1)
- Detect abnormal load change
- Check vibration level at the same time
- High load + low vibration ⇒ **real stress**
- High vibration ⇒ **ignore false alarm**

Output is printed on **Serial Monitor only**.

---

## Repository Structure
