/*
  =====================================================================
  Smart Exhaust Fan Controller — ESP-01 / ESP-01S
  =====================================================================
  Fitur:
    - Relay ON/OFF exhaust fan
    - Servo 180 derajat pengarah aliran udara (Intake / Exhaust)
    - Touch sensor (TTP223) sebagai kontrol lokal:
        * Short press -> toggle relay
        * Long press  -> flip arah airflow (override lokal, tidak
                         mengubah mode manual/auto)
    - Servo bergerak SMOOTH & NON-BLOCKING (bertahap 1 derajat per step,
      bukan langsung lompat), dan relay otomatis OFF sesaat selama servo
      bergerak (safety), lalu balik ke state semula setelah gerakan selesai.
    - Web server (AJAX, tanpa refresh) sebagai antarmuka kontrol
    - NTP time sync untuk mode Auto Schedule
    - Konfigurasi disimpan permanen di LittleFS (config.json)
    - WiFiManager: setting WiFi lewat captive portal, tanpa hardcode
    - ElegantOTA: update firmware & filesystem lewat browser (/update)

  CATATAN SERVO (PENTING):
    Library Servo.h ESP8266 menganggap nilai .write() di bawah 200 sebagai
    DERAJAT, dan 200 ke atas sebagai MICROSECOND PULSA LANGSUNG. Karena itu
    jangan pernah set sudut lebih dari 180 (mis. 270) di ANGLE_INTAKE /
    ANGLE_EXHAUST / lewat slider -- nilai seperti itu akan disalahartikan
    sebagai pulsa mentah (270us), jauh di luar rentang servo normal, dan
    servo akan diam/nyentak aneh. Servo hobi standar cuma didesain 0-180
    derajat secara fisik.
    Kalau servo terasa cuma bergerak sebagian (mis. ~90 derajat) padahal
    target 0-180 penuh, itu karena rentang pulsa default (544-2400us)
    kurang lebar untuk servo tersebut. Sudah dilebarkan ke SERVO_MIN_PULSE_US
    / SERVO_MAX_PULSE_US di bawah -- kalibrasi lagi manual kalau masih kurang
    pas (lihat README).

  Mapping GPIO (ESP-01):
    GPIO0       -> Relay
    GPIO1 (TX)  -> Servo   (Serial TIDAK dipakai karena pin ini dipakai servo)
    GPIO2       -> Touch sensor (TTP223)

  PERINGATAN HARDWARE:
    GPIO0 dan GPIO2 adalah boot-strapping pin ESP8266 (harus HIGH saat
    power-up supaya boot normal, bukan masuk flashing mode). Pastikan
    relay module & touch sensor tidak menahan pin tsb LOW saat device
    baru dinyalakan. Kalau device sering gagal boot / tidak connect,
    cek dulu wiring di 2 pin ini.

  Library yang dibutuhkan (install via Library Manager):
    - ArduinoJson (versi 6.x)
    - WiFiManager (by tzapu)
    - ElegantOTA  (by Ayush Sharma) -- JANGAN pakai AsyncElegantOTA,
      sudah deprecated. ElegantOTA mode default-nya SYNC dan langsung
      kompatibel dengan ESP8266WebServer yang dipakai di sini, tidak
      perlu ESPAsyncWebServer/ESPAsyncTCP sama sekali.
    - ESP8266 Core sudah menyediakan: ESP8266WiFi, ESP8266WebServer,
      DNSServer, LittleFS, Servo, time.h

  PERINGATAN FLASH (ESP-01 = 1MB):
    WiFiManager + ElegantOTA + ArduinoJson + LittleFS + web UI itu
    lumayan berat buat flash 1MB. Kalau upload gagal dengan pesan
    "sketch too big", lihat README bagian pemilihan skema partisi.

  Cara upload:
    1. Pilih board "Generic ESP8266 Module", flash size sesuai modul
       (ESP-01 = 1MB), dan pilih skema partisi yang menyediakan ruang
       OTA + filesystem (misal "1M (FS:64KB OTA:~470KB)" -- nama
       opsi bisa beda tergantung versi board package).
    2. Upload sketch ini seperti biasa (via kabel, hanya perlu sekali).
    3. Upload folder data/ (berisi index.html) ke LittleFS:
       - Arduino IDE 1.8.x: pakai plugin "ESP8266 LittleFS Data Upload"
       - Arduino IDE 2.x  : pakai plugin "arduino-littlefs-upload"
       Tanpa langkah ini, halaman web TIDAK akan muncul (404).
    4. Setelah itu, update firmware/filesystem berikutnya bisa lewat
       OTA (tanpa kabel) lewat http://<ip-esp>/update.
  =====================================================================
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ElegantOTA.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <time.h>

// ============================================================
// KONFIGURASI WIFIMANAGER
// Kalau belum pernah setting WiFi (atau habis reset), ESP akan
// membuka Access Point dengan nama & password di bawah ini.
// Konek ke AP itu dari HP, browser otomatis kebuka captive portal,
// pilih WiFi rumah + masukkan passwordnya. Kredensial disimpan
// permanen, jadi ini cuma perlu dilakukan sekali.
// ============================================================
const char* WIFI_AP_NAME     = "ExhaustFan-Setup";
const char* WIFI_AP_PASSWORD = "exhaustfan123";   // min. 8 karakter
#define WIFI_CONFIG_PORTAL_TIMEOUT_SEC 180   // 3 menit, lalu lanjut tanpa WiFi

// ============================================================
// KONFIGURASI GPIO
// ============================================================
#define RELAY_PIN 0   // GPIO0
#define SERVO_PIN 1   // GPIO1 (TX)
#define TOUCH_PIN 2   // GPIO2

// ============================================================
// KONFIGURASI RELAY
// Modul relay ESP-01 umumnya active LOW (relay ON saat pin LOW).
// Kalau ternyata kebalik (fan nyala pas kondisi "OFF"), tinggal
// ganti LOW -> HIGH di bawah ini.
// ============================================================
#define RELAY_ACTIVE_LEVEL LOW

// ============================================================
// KONFIGURASI TOUCH SENSOR (TTP223)
// Default pabrik TTP223 (tanpa solder jumper) = momentary,
// output HIGH saat disentuh, LOW saat idle.
//
// Kalau kamu tidak yakin module-nya toggle atau momentary, dan
// active HIGH atau LOW: nyalakan device, buka halaman web -> lihat
// field "touchRaw" di status. Idle = nilai saat TIDAK disentuh.
// Kalau saat disentuh nilainya berubah sebentar lalu balik lagi
// sendiri begitu dilepas -> MOMENTARY. Kalau nilainya berubah dan
// TETAP sampai disentuh lagi -> TOGGLE. Sesuaikan define di bawah.
// ============================================================
#define TOUCH_MODE_MOMENTARY 0
#define TOUCH_MODE_TOGGLE    1
#define TOUCH_MODE            TOUCH_MODE_MOMENTARY   // final: momentary
#define TOUCH_ACTIVE_LEVEL    LOW                     // final: active LOW

#define LONG_PRESS_MS  800   // ambang batas long-press (ms)
#define DEBOUNCE_MS    40    // debounce (ms)

// ============================================================
// KONFIGURASI SERVO / AIRFLOW
// ============================================================
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180     // JANGAN diubah lebih dari 180 (lihat catatan di atas)
#define ANGLE_INTAKE    0
#define ANGLE_EXHAUST   180

// Rentang pulsa (microsecond) yang dikirim ke servo untuk 0..180 derajat.
// Default library cuma 544-2400us dan sering tidak mencapai ujung fisik
// servo. 500-2500 lebih lebar dan biasanya bikin sapuan lebih penuh.
// Kalau masih belum pas (misal 0 derajat belum benar-benar mentok, atau
// servo bergetar/berdengung di salah satu ujung), kalibrasi manual:
// gunakan slider di web UI ke posisi 0 dan 180, amati posisi fisiknya,
// lalu sesuaikan angka di bawah (naikkan SERVO_MAX_PULSE_US kalau 180
// belum mentok, turunkan SERVO_MIN_PULSE_US kalau 0 belum mentok --
// tapi jangan sampai servo bergetar terus, itu tandanya kelewat batas).
#define SERVO_MIN_PULSE_US 500
#define SERVO_MAX_PULSE_US 2500

// Kecepatan gerak smooth: makin kecil intervalnya, makin cepat.
// 15ms/derajat -> sapuan penuh 0-180 derajat sekitar 2.7 detik.
#define SERVO_STEP_INTERVAL_MS 15

// ============================================================
// KONFIGURASI NTP
// ============================================================
const char* NTP_SERVER    = "pool.ntp.org";
const long  GMT_OFFSET_SEC = 7 * 3600;  // WIB (UTC+7)
const int   DST_OFFSET_SEC = 0;

// ============================================================
// STORAGE
// ============================================================
const char* CONFIG_PATH = "/config.json";

// ============================================================
// OBJEK GLOBAL
// ============================================================
Servo fanServo;
ESP8266WebServer server(80);
WiFiManager wm;

// ============================================================
// STATE RUNTIME
// ============================================================
bool relayState = false;
int  servoAngle = ANGLE_INTAKE;       // posisi AKTUAL servo saat ini (update tiap step)
int  servoTargetAngle = ANGLE_INTAKE; // posisi TUJUAN servo
bool servoMoving = false;
unsigned long lastServoStepMs = 0;
bool autoMode   = false;          // false = manual, true = auto schedule
int  schedStartH = 8,  schedStartM = 0;   // mulai Exhaust
int  schedEndH   = 22, schedEndM   = 0;   // selesai Exhaust (balik ke Intake)

// state internal untuk debounce & deteksi touch
bool lastRawReading   = false;
bool touchActiveState = false;
unsigned long lastDebounceMs = 0;
unsigned long touchStartMs   = 0;

unsigned long lastSchedulerCheckMs = 0;

// ============================================================
// FORWARD DECLARATIONS (fungsi aksi)
// ============================================================
void setRelay(bool on, bool saveConfig = true);
void applyRelayPin();
void requestServoAngle(int angle, bool saveConfig = true);
void snapServoTo(int angle);
void servoUpdate();
void saveConfigToFS();
void loadConfigFromFS();
void onShortPress();
void onLongPress();
void checkScheduler();
void connectWiFi();
void handleWiFiReset();

// ============================================================
// SETUP
// ============================================================
void setup() {
  // Inisialisasi relay ke posisi aman (OFF) sesegera mungkin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, !RELAY_ACTIVE_LEVEL);

  pinMode(TOUCH_PIN, INPUT);

  fanServo.attach(SERVO_PIN, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);

  LittleFS.begin();
  loadConfigFromFS();

  // Terapkan state hasil load ke hardware. Servo di-snap langsung (tanpa
  // animasi) karena posisi fisik sebenarnya saat power-off tidak diketahui
  // -- tidak ada gunanya animasi dari tebakan posisi awal yang salah.
  snapServoTo(servoAngle);
  setRelay(relayState, false);

  connectWiFi();

  configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER);

  // ---------------- ROUTES ----------------
  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/relay", HTTP_POST, handleSetRelay);
  server.on("/servo", HTTP_POST, handleSetServo);
  server.on("/airflow", HTTP_POST, handleSetAirflowPreset);
  server.on("/mode", HTTP_POST, handleSetMode);
  server.on("/schedule", HTTP_POST, handleSetSchedule);
  server.on("/wifi/reset", HTTP_POST, handleWiFiReset);
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });

  ElegantOTA.begin(&server);   // aktifkan portal update di http://<ip>/update
  server.begin();
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  server.handleClient();
  ElegantOTA.loop();
  handleTouch();
  servoUpdate();

  if (millis() - lastSchedulerCheckMs > 5000) {
    lastSchedulerCheckMs = millis();
    checkScheduler();
  }
}

// ============================================================
// WIFI (WiFiManager)
// ============================================================
void connectWiFi() {
  wm.setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT_SEC);

  // Kalau sudah pernah setting sebelumnya, ini akan langsung connect
  // otomatis tanpa membuka portal. Kalau belum / kredensial gagal,
  // ESP membuka Access Point WIFI_AP_NAME selama batas waktu di atas,
  // lalu lanjut boot tanpa WiFi kalau tidak ada yang setting (device
  // tetap bisa dipakai lokal lewat touch sensor).
  wm.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD);
}

// Dipanggil dari web UI untuk hapus kredensial WiFi tersimpan dan
// restart -- device akan membuka portal setup lagi setelah restart.
void handleWiFiReset() {
  server.send(200, "text/plain", "Mereset WiFi & restart...");
  delay(300);
  wm.resetSettings();
  delay(300);
  ESP.restart();
}

// ============================================================
// AKSI RELAY & SERVO
// ============================================================

// Pin relay fisik selalu dihitung dari relayState (mau ON/OFF) DAN
// apakah servo sedang bergerak. Selama servo bergerak, fan otomatis
// OFF sesaat (safety), lalu balik sesuai relayState begitu selesai --
// dipanggil ulang tiap kali salah satu dari dua kondisi itu berubah.
void applyRelayPin() {
  bool physicalOn = relayState && !servoMoving;
  digitalWrite(RELAY_PIN, physicalOn ? RELAY_ACTIVE_LEVEL : !RELAY_ACTIVE_LEVEL);
}

void setRelay(bool on, bool saveConfig) {
  relayState = on;
  applyRelayPin();
  if (saveConfig) saveConfigToFS();
}

// Minta servo bergerak ke sudut tertentu secara SMOOTH & NON-BLOCKING.
// Pergerakan sebenarnya dieksekusi bertahap oleh servoUpdate() di loop().
void requestServoAngle(int angle, bool saveConfig) {
  angle = constrain(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  servoTargetAngle = angle;

  if (servoTargetAngle != servoAngle && !servoMoving) {
    servoMoving = true;
    applyRelayPin();   // fan OFF sesaat, mulai bergerak
  }

  if (saveConfig) saveConfigToFS();
}

// Langsung set posisi servo TANPA animasi (dipakai cuma saat boot,
// karena posisi fisik sebelum device menyala tidak diketahui).
void snapServoTo(int angle) {
  angle = constrain(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  servoAngle = angle;
  servoTargetAngle = angle;
  servoMoving = false;
  fanServo.write(servoAngle);
}

// Dipanggil tiap loop(). Menggerakkan servo 1 derajat setiap
// SERVO_STEP_INTERVAL_MS menuju servoTargetAngle, sampai sampai tujuan.
void servoUpdate() {
  if (!servoMoving) return;
  if (millis() - lastServoStepMs < SERVO_STEP_INTERVAL_MS) return;
  lastServoStepMs = millis();

  if (servoAngle < servoTargetAngle) servoAngle++;
  else if (servoAngle > servoTargetAngle) servoAngle--;

  fanServo.write(servoAngle);

  if (servoAngle == servoTargetAngle) {
    servoMoving = false;
    applyRelayPin();     // gerakan selesai, fan balik ke relayState semula
    saveConfigToFS();
  }
}

// ============================================================
// TOUCH HANDLING
// ============================================================
void handleTouch() {
#if TOUCH_MODE == TOUCH_MODE_MOMENTARY
  handleTouchMomentary();
#else
  handleTouchToggle();
#endif
}

// Mode momentary: ukur berapa lama pin dalam kondisi "aktif" untuk
// membedakan short-press vs long-press.
void handleTouchMomentary() {
  bool raw = digitalRead(TOUCH_PIN);
  bool pressedRaw = (raw == TOUCH_ACTIVE_LEVEL);

  if (pressedRaw != lastRawReading) {
    lastDebounceMs = millis();
    lastRawReading = pressedRaw;
  }

  if ((millis() - lastDebounceMs) > DEBOUNCE_MS) {
    if (pressedRaw != touchActiveState) {
      touchActiveState = pressedRaw;
      if (touchActiveState) {
        touchStartMs = millis();
      } else {
        unsigned long duration = millis() - touchStartMs;
        if (duration >= LONG_PRESS_MS) {
          onLongPress();
        } else {
          onShortPress();
        }
      }
    }
  }
}

// Mode toggle: module TTP223 sendiri yang membalik output tiap kali
// disentuh, sehingga durasi sentuh tidak bisa dipakai untuk bedakan
// short/long press. Setiap perubahan state diperlakukan sebagai
// short-press (toggle relay). Fitur long-press (flip airflow) TIDAK
// tersedia dalam mode ini -- gunakan tombol di web UI untuk flip airflow.
void handleTouchToggle() {
  static bool lastStable = false;
  bool raw = digitalRead(TOUCH_PIN);

  if (raw != lastRawReading) {
    lastDebounceMs = millis();
    lastRawReading = raw;
  }

  if ((millis() - lastDebounceMs) > DEBOUNCE_MS) {
    if (raw != lastStable) {
      lastStable = raw;
      onShortPress();
    }
  }
}

void onShortPress() {
  setRelay(!relayState);
}

void onLongPress() {
  // Override lokal: flip arah airflow tanpa mengubah mode manual/auto.
  // Jika sedang auto mode, scheduler berikutnya bisa saja menimpa
  // kembali sesuai jadwal -- ini sesuai desain (override sementara).
  int target = (servoAngle >= (SERVO_MAX_ANGLE / 2)) ? ANGLE_INTAKE : ANGLE_EXHAUST;
  requestServoAngle(target);
}

// ============================================================
// SCHEDULER (AUTO MODE)
// ============================================================
void checkScheduler() {
  if (!autoMode) return;

  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  if (t->tm_year < 100) return;  // waktu belum sinkron NTP

  int nowMinutes   = t->tm_hour * 60 + t->tm_min;
  int startMinutes = schedStartH * 60 + schedStartM;
  int endMinutes   = schedEndH   * 60 + schedEndM;

  bool exhaustTime;
  if (startMinutes < endMinutes) {
    exhaustTime = (nowMinutes >= startMinutes && nowMinutes < endMinutes);
  } else {
    // interval melewati tengah malam (misal start 22:00, end 08:00)
    exhaustTime = (nowMinutes >= startMinutes || nowMinutes < endMinutes);
  }

  int targetAngle = exhaustTime ? ANGLE_EXHAUST : ANGLE_INTAKE;
  if (targetAngle != servoTargetAngle) {
    requestServoAngle(targetAngle);
  }
}

// ============================================================
// STORAGE (LittleFS + JSON)
// ============================================================
void loadConfigFromFS() {
  if (!LittleFS.exists(CONFIG_PATH)) return;

  File f = LittleFS.open(CONFIG_PATH, "r");
  if (!f) return;

  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return;

  relayState  = doc["relay"]       | false;
  servoAngle  = doc["servoAngle"]  | ANGLE_INTAKE;
  autoMode    = doc["autoMode"]    | false;
  schedStartH = doc["schedStartH"] | 8;
  schedStartM = doc["schedStartM"] | 0;
  schedEndH   = doc["schedEndH"]   | 22;
  schedEndM   = doc["schedEndM"]   | 0;
}

void saveConfigToFS() {
  StaticJsonDocument<512> doc;
  doc["relay"]       = relayState;
  doc["servoAngle"]  = servoAngle;
  doc["autoMode"]    = autoMode;
  doc["schedStartH"] = schedStartH;
  doc["schedStartM"] = schedStartM;
  doc["schedEndH"]   = schedEndH;
  doc["schedEndM"]   = schedEndM;

  File f = LittleFS.open(CONFIG_PATH, "w");
  if (!f) return;
  serializeJson(doc, f);
  f.close();
}

// ============================================================
// WEB HANDLERS
// ============================================================
void handleRoot() {
  File f = LittleFS.open("/index.html", "r");
  if (!f) {
    server.send(500, "text/plain", "index.html tidak ditemukan di LittleFS. Upload folder data/ dulu.");
    return;
  }
  server.streamFile(f, "text/html");
  f.close();
}

void handleStatus() {
  StaticJsonDocument<512> doc;
  doc["relay"]        = relayState;
  doc["servoAngle"]   = servoAngle;
  doc["servoMoving"]  = servoMoving;
  doc["autoMode"]     = autoMode;
  doc["schedStartH"]  = schedStartH;
  doc["schedStartM"]  = schedStartM;
  doc["schedEndH"]    = schedEndH;
  doc["schedEndM"]    = schedEndM;
  doc["touchRaw"]     = digitalRead(TOUCH_PIN);  // untuk kalibrasi polaritas TTP223
  doc["wifiConnected"]= (WiFi.status() == WL_CONNECTED);
  doc["rssi"]         = WiFi.RSSI();

  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  char timeStr[9];
  if (t->tm_year >= 100) {
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
  } else {
    strcpy(timeStr, "--:--:--");
  }
  doc["time"] = timeStr;

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleSetRelay() {
  if (!server.hasArg("state")) { server.send(400, "text/plain", "missing state"); return; }
  setRelay(server.arg("state") == "on");
  server.send(200, "text/plain", "OK");
}

void handleSetServo() {
  if (!server.hasArg("angle")) { server.send(400, "text/plain", "missing angle"); return; }
  autoMode = false;   // slider manual otomatis pindah ke mode manual
  requestServoAngle(server.arg("angle").toInt(), false);
  saveConfigToFS();
  server.send(200, "text/plain", "OK");
}

void handleSetAirflowPreset() {
  if (!server.hasArg("preset")) { server.send(400, "text/plain", "missing preset"); return; }
  String p = server.arg("preset");
  autoMode = false;
  if (p == "intake")       requestServoAngle(ANGLE_INTAKE, false);
  else if (p == "exhaust") requestServoAngle(ANGLE_EXHAUST, false);
  saveConfigToFS();
  server.send(200, "text/plain", "OK");
}

void handleSetMode() {
  if (!server.hasArg("value")) { server.send(400, "text/plain", "missing value"); return; }
  autoMode = (server.arg("value") == "auto");
  saveConfigToFS();
  server.send(200, "text/plain", "OK");
}

void handleSetSchedule() {
  if (!server.hasArg("start") || !server.hasArg("end")) {
    server.send(400, "text/plain", "missing start/end");
    return;
  }
  int sh, sm, eh, em;
  if (sscanf(server.arg("start").c_str(), "%d:%d", &sh, &sm) != 2) {
    server.send(400, "text/plain", "format start harus HH:MM");
    return;
  }
  if (sscanf(server.arg("end").c_str(), "%d:%d", &eh, &em) != 2) {
    server.send(400, "text/plain", "format end harus HH:MM");
    return;
  }
  schedStartH = sh; schedStartM = sm;
  schedEndH   = eh; schedEndM   = em;
  saveConfigToFS();
  server.send(200, "text/plain", "OK");
}
