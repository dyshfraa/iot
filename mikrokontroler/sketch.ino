const int ledPin = 2; // Ganti sesuai dengan pin yang Anda gunakan
const int buttonPin = 21; // Ganti sesuai dengan pin yang Anda gunakan

int buttonState = 0; // Variabel untuk menyimpan status tombol

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
  buttonState = digitalRead(buttonPin); // Baca status tombol
  
  if (buttonState == HIGH) { // Tombol ditekan
    digitalWrite(ledPin, HIGH); // Hidupkan LED
  } else {
    digitalWrite(ledPin, LOW); // Matikan LED
  }
}
