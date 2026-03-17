#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include <MPU6050.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

#define ss 5
#define rst 14
#define dio0 2
#define buzzer 13
#define mq135Pin 34

// 🔹 Added push button pin
#define buttonPin 4   // Button connected between GPIO 4 and GND

MPU6050 mpu;
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

const float fallThreshold = 3;       // Adjust after testing
const int gasThreshold = 3000;       // MQ135 gas threshold
bool emergencyActive = false;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000;  // 10 seconds
bool buzzerTriggered = false;  // Prevent buzzer from beeping every loop

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17); // GPS RX=16, TX=17

  Serial.println("=======================================");
  Serial.println("🚀 LoRa + MPU6050 + MQ135 + GPS Transmitter");
  Serial.println("=======================================");

  // LoRa setup
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(433E6)) {
    Serial.println("LoRa init...");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  LoRa.setTxPower(20); // max power
  Serial.println("✅ LoRa Initialized OK!");

  // IMU setup
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("❌ MPU6050 connection failed!");
    while (1);
  }

  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);

  // 🔹 Initialize button with internal pull-up
  pinMode(buttonPin, INPUT_PULLUP);
}

// Function to send emergency packet
void sendEmergency(bool fallDetected, bool gasDetected) {
  // Read GPS data
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  String latitude = gps.location.isValid() ? String(gps.location.lat(), 6) : "N/A";
  String longitude = gps.location.isValid() ? String(gps.location.lng(), 6) : "N/A";
  String satellites = String(gps.satellites.value());
  String dateStr = gps.date.isValid()
                     ? String(gps.date.day()) + "/" + String(gps.date.month()) + "/" + String(gps.date.year())
                     : "N/A";
  String timeStr = gps.time.isValid()
                     ? (String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()))
                     : "N/A";

  // --- Send LoRa packet ---
  LoRa.beginPacket();
  LoRa.println("🚨 EMERGENCY SOS!");
  if (fallDetected) LoRa.println("Fall Detected!");
  if (gasDetected) LoRa.println("Hazardous Gas Detected!");
  LoRa.print("Latitude: "); LoRa.println(latitude);
  LoRa.print("Longitude: "); LoRa.println(longitude);
  LoRa.print("Satellites: "); LoRa.println(satellites);
  LoRa.print("Date: "); LoRa.println(dateStr);
  LoRa.print("Time: "); LoRa.println(timeStr);
  LoRa.endPacket();

  Serial.println("📡 SOS Sent, waiting for ACK...");

  // Wait for ACK
  if (waitForAck()) {
    Serial.println("✅ ACK Received!");
  } else {
    Serial.println("⚠️ No ACK received, retrying...");
    delay(2000);
    // Retry sending once
    LoRa.beginPacket();
    LoRa.println("🚨 RETRY SOS!");
    if (fallDetected) LoRa.println("Fall Detected!");
    if (gasDetected) LoRa.println("Hazardous Gas Detected!");
    LoRa.print("Latitude: "); LoRa.println(latitude);
    LoRa.print("Longitude: "); LoRa.println(longitude);
    LoRa.endPacket();
  }
}

// Function to wait for ACK from receiver
bool waitForAck() {
  unsigned long start = millis();
  while (millis() - start < 20000) { // 2 seconds wait
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String resp = "";
      while (LoRa.available()) resp += (char)LoRa.read();
      if (resp.indexOf("ACK") != -1) {
        return true;
      }
    }
  }
  return false;
}

void loop() {
  // --- Read IMU data ---
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  float accX = ax / 16384.0;
  float accY = ay / 16384.0;
  float accZ = az / 16384.0;
  float magnitude = sqrt(accX * accX + accY * accY + accZ * accZ);

  // --- Read Gas Sensor ---
  int gasValue = analogRead(mq135Pin);

  // --- Read GPS data continuously ---
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Prepare GPS info for Serial output
  bool fix = gps.location.isValid();
  int sats = gps.satellites.value();

  // --- Print everything to Serial ---
  Serial.print("Acc: "); Serial.print(magnitude, 2);
  Serial.print(" | Gas: "); Serial.print(gasValue);
  Serial.print(" | Fix: "); Serial.print(fix ? "✅" : "❌");
  Serial.print(" | Sats: "); Serial.println(sats);

  // --- Detect emergencies ---
  bool fallDetected = (magnitude > fallThreshold);
  bool gasDetected = (gasValue > gasThreshold);

  // Trigger buzzer only when threshold crosses
  if ((fallDetected || gasDetected) && !buzzerTriggered) {
    Serial.println("🚨 Threshold crossed! Activating buzzer...");
    
    // 🔹 Added: Sound buzzer for 5 seconds, allow cancel
    digitalWrite(buzzer, HIGH);
    unsigned long startTime = millis();
    bool cancel = false;

    while (millis() - startTime < 5000) {  // 5 seconds window
      if (digitalRead(buttonPin) == LOW) {  // Button pressed (LOW)
        Serial.println("🟢 Worker cancelled false alert!");
        cancel = true;
        break;
      }
      delay(50);
    }

    digitalWrite(buzzer, LOW); // Stop buzzer after 5 sec or cancel

    // 🔹 Only trigger emergency if not cancelled
    if (!cancel) {
      buzzerTriggered = true;
      emergencyActive = true;
      Serial.println("⚠️ No cancel button pressed — sending SOS...");
    } else {
      buzzerTriggered = false;
      emergencyActive = false;
    }
  }

  // Reset buzzer trigger if conditions return to normal
  if (!fallDetected && !gasDetected) {
    buzzerTriggered = false;
  }

  // If in emergency mode, send SOS every 10 seconds
  if (emergencyActive && millis() - lastSendTime > sendInterval) {
    sendEmergency(fallDetected, gasDetected);
    lastSendTime = millis();
  }

  delay(1000);
}
