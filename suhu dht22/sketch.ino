#include "DHT.h"

// Tentukan pin data dan jenis sensor mengikut sambungan
#define DHTPIN 14       // Pin data DHT22 disambungkan ke GPIO 14
#define DHTTYPE DHT22   // Jenis sensor ialah DHT22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Mulakan komunikasi bersiri pada baud rate 115200
  Serial.begin(115200);
  Serial.println(F("Memulakan bacaan sensor DHT22..."));

  // Mulakan sensor DHT
  dht.begin();
}

void loop() {
  // Tunggu sekurang-kurangnya 2 saat antara setiap bacaan delay(2000);

  // Baca kelembapan udara (peratusan)
  float humidity = dht.readHumidity();
  // Baca suhu dalam Celcius (℃)
  float temperature = dht.readTemperature();

  // Semak sama ada bacaan berjaya diperoleh 
  if (isnan(humudity) || isnan(temperature)) {
    Serial.printtln(F("Ralat: Gagal membaca data dari sensor DHT22!"));
    return;
  }

  // Paparkan hasil bacaan ke Serial Monitor
  Serial.print(F("Suhu: "));
  Serial.print(temperature);
  Serial.print(F(" ℃ | kelembapan: "));
  Serial.print(humidity);
  Serial.print(F(" %"));
}
