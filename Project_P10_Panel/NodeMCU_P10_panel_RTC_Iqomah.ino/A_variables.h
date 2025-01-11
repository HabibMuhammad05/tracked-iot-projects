//--------------------------------------------------------Wifi Setup------------------------------------//
//const char* ssid     = "ANANG -2.4G";     
//const char* password = "umikalsum41169";  
 
// Sebagai Station
const char* wifissid = "ANANG -2.4G"; 
const char* wifipassword = "umikalsum41169";

// Sebagai AccessPoint
const char* ssid = "P10_JWS_SETUP"; 
const char* password = "88888888";

IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress netmask(255, 255, 255, 0);

//------------------------------------------------------DMDESP Lib--------------------------------------//
#define DISPLAYS_WIDE 1 // Kolom Panel
#define DISPLAYS_HIGH 1 // Baris Panel
DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);

//--------------------------------------------------Variable for Time-----------------------------------//
int _day, _month, _year, _hour24, _hour12, _minute, _second, _dtw;
int hr24;
float celsius;
String st;
char nameoftheday[7][12] = {"Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu", "Ahad"};
char month_name[12][12] = {"Januari","Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"};
static uint8_t ijadwal;
uint8_t tipealarm;

//------------------------------------------------Variable for prayerTimes------------------------------//
float lati = -7.0613816;
float longi = 110.4441052;
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

//--------------------------------------------------------BUZZER----------------------------------------//
const int buzzer = LED_BUILTIN;
static boolean beep;
bool setCnt = true;

//---------------------------------------------Variables for Setup Webserver----------------------------//
String T1_SSID, T2_PASS, T3_latitude, T4_longitude;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

//---------------------------------------------Variables for IQOMAH----------------------------//
String PesanTeks = "LOREM IPSUM DOLOR SIT AMET";

int detikiqmh;
int menitiqmh;
int durasiSholat = 1; // menit

uint8_t iqmhs = 1; // menit
uint8_t iqmhd = 1; // menit
uint8_t iqmha = 1; // menit
uint8_t iqmhm = 1; // menit
uint8_t iqmhi = 1; // menit
