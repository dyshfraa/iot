#define TRIG_PIN 5
#define ECHO_PIN 18
#define LED_PIN 14

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT); // TRIG memancarkan suara
  pinMode(ECHO_PIN, INPUT); // ECHO mendengarkan pantulan
  pinMode(LED_PIN, OUTPUT); // konfigurasi pin LED sebagai OUTPUT

  digitalWrite(LED_PIN, LOW); // Pastikan LED mati saat awal boot
}

void loop() {
  // 1. Memancarkan gelombang suara selama 10 microdetik
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 2. Membaca waktu pantulan (dalan microdetik)
  long duration = pulseIn(ECHO_PIN, HIGH);

  // 3. Menghitung jarak ke centimeter
  float distance = duration * 0.034 / 2;

  // 4. Logika kontrol LED Merah
  if (distance < 20.0) {
    digitalWrite(LED_PIN, HIGH); // LED Merah MENYALA
  } else {
    digitalWrite(LED_PIN, LOW); // LED Merah MATI
  }
  

  Serial.print("Jarak: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(1000); // Jeda 1 detik anatr pembaca
}
