#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin Driver Motor
#define IN1 12
#define IN2 14 //
#define IN3 27 //
#define IN4 26 //
#define ENA 13 //kiri 
#define ENB 25 //kanan

// Pin Sensor
#define SENSOR_KIRI 34
#define SENSOR_TENGAH 39
#define SENSOR_KANAN 36

// Pin Tombol
#define TOMBOL_JALAN_BERHENTI 16
#define TOMBOL_KECEPATAN 17

int pwmFrequency = 5000;
int ENA_CHANNEL = 0;
int ENB_CHANNEL = 1;
int pwmResolution = 8;

// Konstanta
int indeksRPM = 100;  // Awal level RPM pada kecepatan terendah

// Inisialisasi Komponen
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variabel
bool robotBerjalan = false; // Status robot bergerak atau berhenti
int errorSebelumnya = 0, integral = 0;
int errorTerakhir = 0;

// Variabel PID
float Kp = 0.8, Ki = 0, Kd = 0.5;

void setup() {
  // Inisialisasi LCD setup 1
//  lcd.init();
//  lcd.backlight();
//  lcd.begin(16,2);
  
  // Inisialisasi LCD setup 1
  lcd.begin();
  lcd.backlight();

  // Inisialisasi Pin
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(TOMBOL_JALAN_BERHENTI, INPUT_PULLUP);
  pinMode(TOMBOL_KECEPATAN, INPUT_PULLUP);

  pinMode(SENSOR_KIRI, INPUT);
  pinMode(SENSOR_TENGAH, INPUT);
  pinMode(SENSOR_KANAN, INPUT);

    // Inisialisasi PWM pada ENA dan ENB (untuk ESP32, kita perlu menggunakan ledcWrite)
//  ledcAttach(ENA, 5000, 8);  // Mengonfigurasi pin ENA untuk PWM dengan frekuensi 5000 Hz dan resolusi 8 bit
//  ledcAttach(ENB, 5000, 8);  // Mengonfigurasi pin ENB untuk PWM dengan frekuensi 5000 Hz dan resolusi 8 bit
  
  ledcSetup(1, 5000, 8);
  ledcAttachPin(ENA, 1);
  ledcSetup(2, 5000, 8);
  ledcAttachPin(ENB, 2);
  
  
  //   ledcWrite(ENA, 255);  // Use ledcWrite for ESP32 PWM control
  //   digitalWrite(IN1, HIGH);
  //   digitalWrite(IN2, LOW);

  // // Motor Kanan
  //   ledcWrite(ENB, 255);  // Use ledcWrite for ESP32 PWM control
  //   digitalWrite(IN3, HIGH);
  //   digitalWrite(IN4, LOW);
  // delay(200); // Tunggu sebentar sebelum mulai
}

void loop() {
  // Handle Input Tombol
  handleTombol();

  if (robotBerjalan) {
    // Membaca Nilai Sensor
    int sensorKiri = analogRead(SENSOR_KIRI);
    int sensorTengah = analogRead(SENSOR_TENGAH);
    int sensorKanan = analogRead(SENSOR_KANAN);

    // Hitung Error
    int error = hitungError(sensorKiri, sensorTengah, sensorKanan);

    // Hitung Output PID
    integral += error;
    int derivatif = error - errorSebelumnya;
    int outputPID = (Kp * error) + (Ki * integral) + (Kd * derivatif);
    errorSebelumnya = error;

    // Hitung Kecepatan Motor
    int kecepatanDasar = indeksRPM; // Kecepatan dasar dari level RPM
    int kecepatanKiri = constrain(kecepatanDasar - outputPID, 0, 255);
    int kecepatanKanan = constrain(kecepatanDasar + outputPID, 0, 255);

    // Menggerakkan Motor
    kendalikanMotor(kecepatanKiri, kecepatanKanan);

    // Menampilkan Status di LCD
    // lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kecepatan: ");
    lcd.print(kecepatanDasar);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print("  MODE  SERIUS  ");
  } else {
    // Hentikan Motor
    kendalikanMotor(0, 0);

    // Menampilkan Status di LCD
    // lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kecepatan: ");
    lcd.print(indeksRPM);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print("stop dulu bos");
  }
}

void handleTombol() {
  static bool statusTombolJalan = HIGH;       // Variabel statis untuk menyimpan status tombol jalan
  static bool statusTombolKecepatan = HIGH;  // Variabel statis untuk menyimpan status tombol kecepatan
  
  // Membaca Status Tombol
  bool tombolJalan = digitalRead(TOMBOL_JALAN_BERHENTI);
  bool tombolKecepatan = digitalRead(TOMBOL_KECEPATAN);

  if (tombolJalan == LOW && statusTombolJalan == HIGH) {
    robotBerjalan = !robotBerjalan; // Menghentikan atau memulai robot
  }
  statusTombolJalan = tombolJalan; // Perbarui status tombol jalan
  
  // Mengatur Kecepatan
  if (tombolKecepatan == LOW && statusTombolKecepatan == HIGH) {
    indeksRPM += 5;
    if (indeksRPM > 256) {
      indeksRPM = 150; // Kembali ke level pertama
    }
  }
  statusTombolKecepatan = tombolKecepatan; // Perbarui status tombol kecepatan
}

//int hitungError(int sensorKiri, int sensorTengah, int sensorKanan) {
//  // Array posisi relatif sensor
//  const int posisi[] = {-1, 0, 1}; // Kiri, Tengah, Kanan
//
//  // Hitung total bobot dan total pembacaan sensor
//  int totalBobot = (sensorKiri * posisi[0]) + (sensorTengah * posisi[1]) + (sensorKanan * posisi[2]);
//  int totalSensor = sensorKiri + sensorTengah + sensorKanan;
//
//  // Penanganan garis hilang
//  if (totalSensor == 0) {
//    return errorTerakhir * 100; // Jika semua sensor tidak mendeteksi garis, gunakan error terakhir
//  }
//
//  // Hitung error proporsional
//  int error = totalBobot / totalSensor;
//
//  // Simpan error terakhir untuk penanganan garis hilang
//  errorTerakhir = error;
//
//  // Scaling untuk tuning PID
//  return error * 100; // Dikembalikan dalam skala yang lebih besar
//}


int hitungError(int sensorKiri, int sensorTengah, int sensorKanan) {
  const int ambangBatas = 500; // Threshold untuk membaca garis
  int error = 0;

//  if (sensorTengah > ambangBatas) {
//    error = 0; // Robot berada di tengah garis
//  } else if (sensorKiri > ambangBatas) {
//    error = -1; // Garis berada di kiri
//  } else if (sensorKanan > ambangBatas) {
//    error = 1; // Garis berada di kanan
//  }

//  return error * 100; // Scaling untuk tuning PID

   if (sensorTengah > ambangBatas) {
    error = 0; // Robot berada di tengah garis
    errorTerakhir = error; // Perbarui error terakhir
  } else if (sensorKiri > ambangBatas) {
    error = -1; // Garis berada di kiri
    errorTerakhir = error; // Perbarui error terakhir
  } else if (sensorKanan > ambangBatas) {
    error = 1; // Garis berada di kanan
    errorTerakhir = error; // Perbarui error terakhir
  } else {
    // Jika semua sensor tidak mendeteksi garis
    error = errorTerakhir; // Gunakan error terakhir
  }

  return error * 100; // Scaling untuk tuning PID
}

void kendalikanMotor(int kecepatanKiri, int kecepatanKanan) {
  // Motor Kiri
  if (kecepatanKiri > 0) {
    ledcWrite(1, kecepatanKiri);
//    ledcWrite(ENA, kecepatanKiri);  // Use ledcWrite for ESP32 PWM control
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else {
    ledcWrite(1, 0);
//    ledcWrite(ENA, 0);  // Turn off PWM if speed is 0
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }

  // Motor Kanan
  if (kecepatanKanan > 0) {
    ledcWrite(2, kecepatanKanan);
//    ledcWrite(ENB, kecepatanKanan);  // Use ledcWrite for ESP32 PWM control
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else {
    ledcWrite(2, 0);
//    ledcWrite(ENB, 0);  // Turn off PWM if speed is 0
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
}
