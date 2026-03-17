#include <LoRa.h>
#include <SPI.h>

#define ss 5
#define rst 14
#define dio0 2
#define ledPin 12
#define buzzerPin 13   // 🔔 Buzzer connected to digital pin 13

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("====================================");
  Serial.println("📡 LoRa Receiver - SOS Alerts with ACK + Buzzer");
  Serial.println("====================================");

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);   // Set buzzer pin as output
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(433E6)) {
    Serial.println("LoRa init...");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("✅ LoRa Initialized OK!\n");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String LoRaData = "";
    while (LoRa.available()) {
      LoRaData += (char)LoRa.read();
    }

    Serial.println("\n===============================");
    Serial.println("🚨 EMERGENCY ALERT RECEIVED");
    Serial.println("===============================");
    Serial.println(LoRaData);
    Serial.println("===============================");

    blinkLED(8); // Visual indicator of alert
    activateBuzzer(5000); // 🔊 Sound buzzer for 5 seconds

    // --- Send ACK back ---
    LoRa.beginPacket();
    LoRa.println("ACK");
    LoRa.endPacket();
    Serial.println("✅ ACK Sent!\n");
  }
}

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(300);
    digitalWrite(ledPin, LOW);
    delay(300);
  }
}

void activateBuzzer(unsigned long duration) {
  digitalWrite(buzzerPin, HIGH);   // Turn ON buzzer
  delay(duration);                 // Keep it ON for 'duration' ms
  digitalWrite(buzzerPin, LOW);    // Turn OFF buzzer
}
