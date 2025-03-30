//========================== TIME CALCULATION ================================//
int h, m, s, NTPday, NTPmonth, NTPyear, NTPdayOfWeek;
long localEpoc = 0;
long localMillisAtUpdate = 0;
String date;
String buf="";
char* monthNames[] = {"JAN","FEB","MAR","APR","MEI","JUN","JUL","AGU","SEP","OKT","NOV","DES"};
char txt[30];

//========================== NETWORK SETTING ================================//
const char* ssid     = "ANANG -2.4G";    
const char* password = "umikalsum41169";   
long utcOffset = 7;

//========================= MAIN MODE CONTROL ================================//
uint32_t updTime;
bool dots;
uint8_t tmpUtama;

//========================= MAX7219 LIB SETUP ================================//
#define NUM_MAX 8
#define LINE_WIDTH 32
#define ROTATE  90

#define CLK_PIN   14
#define DIN_PIN  13
#define CS_PIN    15

int xPos=0, yPos=0;
