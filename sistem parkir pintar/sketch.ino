#define TRIG_PIN 5
#define ECHO_PIN 18

#define LED_HIJAU  19
#define LED_KUNING 21
#define LED_MERAH  22
#define BUZZER_PIN 23

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  // Trigger pulsa ultrasonik
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Hitung durasi dan jarak (cm)
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;

  Serial.print("Jarak: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Logika 3 Kondisi Parkir
  if (distance > 60) {
    // Kondisi Aman
    digitalWrite(LED_HIJAU, HIGH);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_MERAH, LOW);
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);

  } else if (distance >= 30 && distance <= 60) {
    // Kondisi Hati-hati (Buzzer bip lambat)
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_KUNING, HIGH);
    digitalWrite(LED_MERAH, LOW);

    tone(BUZZER_PIN, 1000);
    delay(150);
    noTone(BUZZER_PIN);
    delay(300);

  } else {
    // Kondisi Bahaya/Stop (< 30 cm - Buzzer terus menyala)
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_MERAH, HIGH);

    tone(BUZZER_PIN, 1000); // Bunyi kontinu
    delay(100);
  }
}
