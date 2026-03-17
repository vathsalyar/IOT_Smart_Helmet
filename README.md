# IoT Smart Helmet for Industrial Worker Safety

An intelligent IoT-based smart helmet designed to improve worker safety in industrial and mining environments by detecting falls and hazardous air quality conditions.

## Features

- ML-based fall detection using MPU6050
- Air quality monitoring using MQ135 gas sensor
- GPS location tracking with NEO-6M module
- Long-range LoRa communication using SX1278
- Local alert system with buzzer
- Supervisor alert system with buzzer and LED
- False alarm cancellation button

## System Architecture

The system consists of three layers:

1. **Sensing Layer**
   - MPU6050 (motion detection)
   - MQ135 (air quality)
   - NEO-6M (GPS location)

2. **Processing Layer**
   - ESP32 microcontroller
   - TensorFlow Lite model for fall detection

3. **Communication Layer**
   - LoRa SX1278
   - Local buzzer alerts
   - Remote supervisor alerts

## Hardware Components

- ESP32
- MPU6050 IMU sensor
- MQ135 Gas sensor
- NEO-6M GPS module
- SX1278 LoRa module
- Buzzer
- Push button
- LED

## Machine Learning Model

- Dataset collected from MPU6050 sensor
- 6-axis IMU features:
  - AccX, AccY, AccZ
  - GyroX, GyroY, GyroZ
- Algorithm used: **Random Forest Classifier**
- Model converted to **TensorFlow Lite** for ESP32 deployment

## Alert System Workflow

1. System continuously monitors:
   - Air quality
   - Motion data

2. If hazard detected:
   - Local buzzer activates

3. Worker can cancel false alarm using push button

4. If not cancelled:
   - LoRa sends alert packet with:
     - Alert type
     - GPS coordinates
     - Timestamp

5. Supervisor receiver:
   - Activates buzzer
   - Turns on LED

## Results

- Fall detection accuracy: **~85%**
- Reliable long-range LoRa communication
- Real-time monitoring and alert system

## Applications

- Industrial worker safety
- Mining safety monitoring
- Construction site safety
- Hazardous environment monitoring
