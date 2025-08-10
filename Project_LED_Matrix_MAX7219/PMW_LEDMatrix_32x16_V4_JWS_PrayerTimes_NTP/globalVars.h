//========================== TIME CALCULATION ================================//
int h, m, s, NTPday, NTPmonth, NTPyear, NTPdayOfWeek;
char monthNames[][5] = {"JAN","FEB","MAR","APR","MEI","JUN","JUL","AGU","SEP","OKT","NOV","DES"};
char namaHari[][7] = {"AHAD", "SENIN", "SELASA", "RABU", "KAMIS", "JUMAT", "SABTU", "AHAD"}; // array hari, dihitung mulai dari senin, hari senin angka nya =0,
char txt[30];

//========================== NETWORK SETTING ================================//
const char* ssid     = "lorem ipsum";    
const char* password = "";   
long utcOffset = 7;

//========================= MAIN MODE CONTROL ================================//
uint32_t updTime;
bool dots;
uint8_t tmpUtama;
static uint16_t d;

//========================= MAX7219 LIB SETUP ================================//
#define NUM_MAX 8
#define LINE_WIDTH 32
#define ROTATE  90

#define CLK_PIN   14
#define DIN_PIN  13
#define CS_PIN    15

int xPos=0, yPos=0;
