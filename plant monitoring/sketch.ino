#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ==========================================
// DEFINISI PIN GPIO ESP32
// ==========================================
#define PIN_SOIL        34    // Input Analog Soil Moisture (Slide Potentiometer)
#define PIN_LDR         35    // Input Analog Modul Sensor Cahaya LDR (Pin AO)
#define PIN_DHT         15    // Input Digital Sensor DHT22
#define DHTTYPE         DHT22

#define PIN_RELAY       26    // Output Relay Pengendali Pompa Air
#define PIN_BUZZER      25    // Output Buzzer Alarm
#define PIN_LED_GREEN   14    // Indikator Tanah Normal (>= 60%)
#define PIN_LED_YELLOW  27    // Indikator Tanah Mulai Kering (30% - 59%)
#define PIN_LED_RED     13    // Indikator Tanah Sangat Kering (< 30%)

// ==========================================
// KONFIGURASI AMBANG BATAS (MUDAH DIUBAH)
// ==========================================
const int   SOIL_LOW_THRESHOLD   = 30;   // < 30%  : Tanah sangat kering -> Pompa ON
const int   SOIL_HIGH_THRESHOLD  = 60;   // >= 60% : Tanah cukup/normal  -> Pompa OFF
const int   SOIL_WET_THRESHOLD   = 85;   // >= 85% : Kategori tanah basah (WET)
const float TEMP_MAX_THRESHOLD   = 35.0; // > 35°C : Peringatan suhu terlalu tinggi
const int   LIGHT_THRESHOLD_PCT  = 50;   // >= 50% : Cahaya Terang (BRIGHT), < 50% : Gelap (DARK)

// ==========================================
// OBJEK PERANGKAT KERAS
// ==========================================
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(PIN_DHT, DHTTYPE);

// ==========================================
// VARIABEL DATA & STATUS SISTEM
// ==========================================
int   soilMoisturePct = 50;
float temperatureC    = 28.0;
float humidityPct     = 65.0;
int   lightIntensity  = 70;

bool  pumpState       = false; // false = OFF, true = ON
bool  sensorError     = false;
bool  highTempAlert   = false;
bool  drySoilAlert    = false;

// Variabel Timer Non-Blocking (millis)
unsigned long lastAnalogMillis = 0;
unsigned long lastDhtMillis    = 0;
unsigned long lastLcdMillis    = 0;
unsigned long lastPageMillis   = 0;
unsigned long lastAlarmMillis  = 0;

int   lcdPage      = 0; // 0: Soil & Temp, 1: Humidity & Light, 2: Pump & System Status
bool buzzerToggle = false;

String prevLine0  = "";
String prevLine1  = "";

// ==========================================
// FUNGSI UPDATE LCD TANPA KEDIP (FLICKER-FREE)
// ==========================================
void updateLcd(const char* l0, const char* l1) {
  char buf0[17];
  char buf1[17];
  snprintf(buf0, sizeof(buf0), "%-16s", l0);
  snprintf(buf1, sizeof(buf1), "%-16s", l1);

  String s0 = String(buf0);
  String s1 = String(buf1);

  if (s0 != prevLine0 || s1 != prevLine1) {
    lcd.setCursor(0, 0);
    lcd.print(s0);
    lcd.setCursor(0, 1);
    lcd.print(s1);
    prevLine0 = s0;
    prevLine1 = s1;
  }
}

// ==========================================
// FUNGSI KLASIFIKASI TEKS STATUS TANAH
// ==========================================
const char* getSoilStatusText(int pct) {
  if (pct < SOIL_LOW_THRESHOLD)  return "VERY DRY";
  if (pct < SOIL_HIGH_THRESHOLD) return "DRYING";
  if (pct < SOIL_WET_THRESHOLD)  return "NORMAL";
  return "WET";
}

// ==========================================
// SETUP AWAL SISTEM
// ==========================================
void setup() {
  Serial.begin(115200);

  // Konfigurasi Resolusi ADC ESP32 (12-bit: 0 - 4095)
  analogReadResolution(12);
  pinMode(PIN_SOIL, INPUT);
  pinMode(PIN_LDR, INPUT);

  // Konfigurasi Pin Output
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);

  // Kondisi Awal Aman
  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED, LOW);
  noTone(PIN_BUZZER);

  // Inisialisasi Sensor DHT22
  dht.begin();

  // Inisialisasi LCD 16x2 I2C
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  updateLcd("SMART PLANT SYS", "INITIALIZING...");
  delay(1500);

  // Pembacaan perdana DHT22
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t) && !isnan(h)) {
    temperatureC = t;
    humidityPct  = h;
    sensorError  = false;
  }

  Serial.println("================================================");
  Serial.println("SMART PLANT MONITORING SYSTEM - ESP32 SIMULATOR");
  Serial.println("================================================");
}

// ==========================================
// LOOP UTAMA (NON-BLOCKING)
// ==========================================
void loop() {
  unsigned long currentMillis = millis();

  // ----------------------------------------------------
  // 1. BACA SENSOR DHT22 (Setiap 2000 ms)
  // ----------------------------------------------------
  if (currentMillis - lastDhtMillis >= 2000) {
    lastDhtMillis = currentMillis;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h) || t < -40.0 || t > 85.0 || h < 0.0 || h > 100.0) {
      sensorError = true;
    } else {
      sensorError  = false;
      temperatureC = t;
      humidityPct  = h;
    }
  }

  // ----------------------------------------------------
  // 2. BACA SENSOR ANALOG & LOGIKA OTOMATISASI (500 ms)
  // ----------------------------------------------------
  if (currentMillis - lastAnalogMillis >= 500) {
    lastAnalogMillis = currentMillis;

    int rawSoil = analogRead(PIN_SOIL);
    int rawLdr  = analogRead(PIN_LDR);

    if (rawSoil < 0 || rawSoil > 4095 || rawLdr < 0 || rawLdr > 4095) {
      sensorError = true;
    }

    soilMoisturePct = constrain(map(rawSoil, 0, 4095, 0, 100), 0, 100);
    lightIntensity  = constrain(map(rawLdr, 4095, 0, 0, 100), 0, 100);

    if (sensorError) {
      pumpState     = false;
      highTempAlert = false;
      drySoilAlert  = false;

      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_YELLOW, LOW);
      digitalWrite(PIN_LED_RED, HIGH);
    } else {
      if (soilMoisturePct < SOIL_LOW_THRESHOLD) {
        pumpState = true;  
      } else if (soilMoisturePct >= SOIL_HIGH_THRESHOLD) {
        pumpState = false; 
      }

      if (soilMoisturePct < SOIL_LOW_THRESHOLD) {
        digitalWrite(PIN_LED_RED, HIGH);
        digitalWrite(PIN_LED_YELLOW, LOW);
        digitalWrite(PIN_LED_GREEN, LOW);
        drySoilAlert = true;
      } else if (soilMoisturePct < SOIL_HIGH_THRESHOLD) {
        digitalWrite(PIN_LED_RED, LOW);
        digitalWrite(PIN_LED_YELLOW, HIGH);
        digitalWrite(PIN_LED_GREEN, LOW);
        drySoilAlert = false;
      } else {
        digitalWrite(PIN_LED_RED, LOW);
        digitalWrite(PIN_LED_YELLOW, LOW);
        digitalWrite(PIN_LED_GREEN, HIGH);
        drySoilAlert = false;
      }

      highTempAlert = (temperatureC > TEMP_MAX_THRESHOLD);
    }

    digitalWrite(PIN_RELAY, pumpState ? HIGH : LOW);

    // Tampilkan Log di Serial Monitor
    Serial.print("[PLANT LOG] Soil: ");
    Serial.print(soilMoisturePct);
    Serial.print("% (");
    Serial.print(getSoilStatusText(soilMoisturePct));
    Serial.print(") | Temp: ");
    if (sensorError) {
      Serial.print("ERR");
    } else {
      Serial.print(temperatureC, 1);
      Serial.print("C | Hum: ");
      Serial.print(humidityPct, 1);
      Serial.print("%");
    }
    Serial.print(" | Light: ");
    Serial.print(lightIntensity);
    Serial.print("% | Pump: ");
    Serial.println(pumpState ? "ON" : "OFF");
  }

  // ----------------------------------------------------
  // 3. KONTROL ALARM BUZZER & LED ERROR (Setiap 350 ms)
  // ----------------------------------------------------
  if (currentMillis - lastAlarmMillis >= 350) {
    lastAlarmMillis = currentMillis;
    buzzerToggle = !buzzerToggle;

    if (sensorError) {
      if (buzzerToggle) {
        tone(PIN_BUZZER, 1800);
        digitalWrite(PIN_LED_RED, HIGH);
      } else {
        noTone(PIN_BUZZER);
        digitalWrite(PIN_LED_RED, LOW);
      }
    } else if (highTempAlert || drySoilAlert) {
      if (buzzerToggle) {
        tone(PIN_BUZZER, 1200);
      } else {
        noTone(PIN_BUZZER);
      }
    } else {
      noTone(PIN_BUZZER);
    }
  }

  // ----------------------------------------------------
  // 4. ROTASI TAMPILAN LCD (Setiap 2500 ms)
  // ----------------------------------------------------
  if (currentMillis - lastPageMillis >= 2500) {
    lastPageMillis = currentMillis;
    lcdPage = (lcdPage + 1) % 3;
  }

  // ----------------------------------------------------
  // 5. UPDATE TAMPILAN LCD BERDASARKAN HALAMAN AKTIF
  // ----------------------------------------------------
  char line0[17];
  char line1[17];

  if (sensorError) {
    updateLcd("SYSTEM ERROR!", "CHECK SENSORS");
  } else {
    if (lcdPage == 0) {
      // Halaman 1: Soil Moisture & Temperature
      snprintf(line0, sizeof(line0), "Soil: %d%% (%s)", soilMoisturePct, getSoilStatusText(soilMoisturePct));
      snprintf(line1, sizeof(line1), "Temp: %.1f C", temperatureC);
    } 
    else if (lcdPage == 1) {
      // Halaman 2: Humidity & Light Intensity
      snprintf(line1, sizeof(line1), "Hum: %.1f%%", humidityPct);
      snprintf(line0, sizeof(line0), "Light: %d%% (%s)", lightIntensity, (lightIntensity >= LIGHT_THRESHOLD_PCT) ? "BRIGHT" : "DARK");
    } 
    else if (lcdPage == 2) {
      // Halaman 3: Pump Status & Alert
      snprintf(line0, sizeof(line0), "Pump: %s", pumpState ? "ACTIVE (ON)" : "STANDBY (OFF)");
      snprintf(line1, sizeof(line1), "Status: %s", highTempAlert ? "HIGH TEMP!" : (drySoilAlert ? "DRY SOIL!" : "NORMAL"));
    }
    updateLcd(line0, line1);
  }
}
