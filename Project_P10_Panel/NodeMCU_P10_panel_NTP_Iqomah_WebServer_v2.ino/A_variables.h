//--------------------------------------------------------Wifi Setup------------------------------------//
const char* ssid     = "JWS";     
const char* password = "12345678";   

String ipAdress;

//------------------------------------------------------DMDESP Lib--------------------------------------//
#define DISPLAYS_WIDE 1 // Kolom Panel
#define DISPLAYS_HIGH 1 // Baris Panel
DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);

//--------------------------------------------------Variable for Time-----------------------------------//
int _day, _month, _year, _hour24, _hour12, _minute, _second, _dtw;
int hr24, jam24;
String st;
char nameoftheday[7][12] = {"AHAD", "SENIN", "SELASA", "RABU", "KAMIS", "JUM'AT", "SABTU"};
char month_name[12][12] = {"JAN","FEB", "MAR", "APR", "MEI", "JUN", "JUL", "AGU", "SEP", "OKT", "NOV", "DES"};
static uint8_t ijadwal;
uint8_t tipealarm;
//------------------------------------------------Variable for Epoch time-------------------------------//
time_t requestSync(){
  Serial.write(7); 
  return 0; 
}

//------------------------------------------------Variable for NTP Clock--------------------------------//
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
unsigned long Epoch_Time; 
uint8_t utcOffset = 7; // Time Zone setting

//------------------------------------------------Variable for prayerTimes------------------------------//
float lati = -7.06138;
float longi = 110.44410;
double times[sizeof(TimeName)/sizeof(char*)];
int ihti = 2;

//--------------------------------------------------Variable for Millis---------------------------------//
const long interval = 1000; 
unsigned long previousMillis = 0;
const long interval_for_date = 60; //-> For scroll speed
unsigned long previousMillis_for_date = 0;
unsigned long prevdateMillis = 0;
unsigned long previousMillisIsr = 0;

unsigned long previousPrint = 0;

//-----------------------------------------Variable to display hours and minutes------------------------//
char hr_24 [3];
String str_hr_24;
char mn [3];
String str_mn;
char sc [3];
String str_sc;

//----------------------------------------------Variable For Scrolling text-----------------------------//
unsigned long dotMillis=0;

int dotInt = 1000;
bool dotBool;
String teks;
bool Clr = true;
bool scrAnim  = true;

uint8_t tmpinfo;
bool mod = true;

//--------------------------------------------------------BUZZER----------------------------------------//
const int buzzer = LED_BUILTIN;
static boolean beep;
bool setCnt = true;

bool norMode = true;

#define tombol_jam A0

//---------------------------------------------Variables for Setup Webserver----------------------------//
String T1_SSID, T2_PASS, T3_latitude, T4_longitude, T5_Catatan;

String Tsholat, Ts, Td, Ta, Tm, Ti;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

//---------------------------------------------Variables for IQOMAH----------------------------//
String PesanTeks = "SHOLATLAH SEBELUM ANDA DISHOLATKAN";

int detikiqmh;
int menitiqmh;
uint8_t durasiSholat = 1; // menit

uint8_t iqmhs = 1; // menit
uint8_t iqmhd = 1; // menit
uint8_t iqmha = 1; // menit
uint8_t iqmhm = 1; // menit
uint8_t iqmhi = 1; // menit
