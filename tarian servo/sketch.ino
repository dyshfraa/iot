#include <ESP32Servo.h>

const int NUM_SERVOS = 12;

// Urutan pin sesuai dengan diagram.json (servo1 sampai servo12)
const int servoPins[NUM_SERVOS] = {
  13, 12, 14, 27, // Baris Atas (servo1 - servo4)
  26, 25,         // Sisi Kiri (servo5 - servo6)
  33, 32,         // Sisi Kanan (servo7 - servo8)
  15, 2, 4, 16    // Baris Bawah (servo9 - servo12)
};

Servo servos[NUM_SERVOS];

void setup() {
  Serial.begin(115200);

  // Mengalokasikan 4 timer PWM pada ESP32 (mendukung hingga 16 servo)
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Inisialisasi setiap servo
  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].setPeriodHertz(50);             // Frekuensi standar servo 50Hz
    servos[i].attach(servoPins[i], 500, 2400); // Min 500us, Max 2400us
    servos[i].write(90);                      // Posisi awal di tengah (90 derajat)
  }

  Serial.println("12 Servo siap dijalankan!");
  delay(1000);
}

void loop() {
  // 1. Gerakan Bersamaan (0 ke 180 derajat)
  Serial.println("Gerakan 1: Sweep Bersamaan");
  for (int pos = 0; pos <= 180; pos += 2) {
    for (int i = 0; i < NUM_SERVOS; i++) {
      servos[i].write(pos);
    }
    delay(15);
  }

  // 2. Gerakan Bersamaan Kembali (180 ke 0 derajat)
  for (int pos = 180; pos >= 0; pos -= 2) {
    for (int i = 0; i < NUM_SERVOS; i++) {
      servos[i].write(pos);
    }
    delay(15);
  }

  delay(500);

  // 3. Gerakan Berurutan (Wave / Gelombang satu per satu)
  Serial.println("Gerakan 2: Gelombang Berurutan");
  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].write(150);
    delay(100);
    servos[i].write(30);
    delay(100);
    servos[i].write(90); // Kembali ke tengah
  }

  delay(500);
}
