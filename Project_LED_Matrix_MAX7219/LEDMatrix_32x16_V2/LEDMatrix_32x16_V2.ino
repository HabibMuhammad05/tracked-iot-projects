
#include <ESP8266WiFi.h>
#include <DHT.h>

#include <PrayerTimes.h>
// PrayerTimes
double times[sizeof(TimeName)/sizeof(char*)];
int ihti = 2;

#define DHTPIN 4     // Update to your sensor's pin
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float temp;
float hum;

unsigned long lastDisplayTime = 0;
bool showTemp = true;

#define NUM_MAX 8
#define LINE_WIDTH 32
#define ROTATE  90

#define CLK_PIN   14
#define DIN_PIN  13
#define CS_PIN    15

//#define DEBUG(x)
#define DEBUG(x) x

int h, m, s, day, month, year, dayOfWeek;
long localEpoc = 0;
long localMillisAtUpdate = 0;
String date;
String buf="";
char* monthNames[] = {"JAN","FEB","MAR","APR","MEI","JUN","JUL","AGU","SEP","OKT","NOV","DES"};
char txt[30];

// =======================================================================
// Your config below!
// =======================================================================
const char* ssid     = "ANANG -2.4G";     // SSID of local network
const char* password = "umikalsum41169";   // Password on network
long utcOffset = 7; // Time Zone setting
// =======================================================================


#include "fonts.h"
#include "max7219.h"
#include "timeCalc.h"


void setup(){
  buf.reserve(500);
  Serial.begin(115200);
  dht.begin();
  
  initMAX7219();
  sendCmdAll(CMD_SHUTDOWN, 1);
  sendCmdAll(CMD_INTENSITY, 0);
  DEBUG(Serial.print("Connecting to WiFi ");)
  WiFi.begin(ssid, password);
  clr();
  xPos=0;
  printString("CONNECT..", font3x7);
  refreshAll();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); DEBUG(Serial.print("."));
  }
  clr();
  xPos=0;
  DEBUG(Serial.println(""); Serial.print("MyIP: "); Serial.println(WiFi.localIP());)
  printString((WiFi.localIP().toString()).c_str(), font3x7);
  refreshAll();
  getTime();
  temp = dht.readTemperature();
  hum = dht.readHumidity();
}

// =======================================================================

void loop()
{
  curTime = millis();
  if(curTime-updTime>600000) {
    updTime = curTime;
    getTime();  // update time every 600s=10m
  }
  dots = (curTime % 1000)<500;     // blink 2 times/sec
  updateTime();
  mode = (curTime / 20000) % 4;  
  switch(mode) {
    case 0: drawTime0(); break;
    case 1: drawTime2(); break;
    case 2: drawTime1(); break;
    case 3: drawTime3(); break;
    case 4: drawTime4(); break;
  }
  refreshAll();
  delay(10);
}

// =======================================================================

// for ESP-01 module
//#define DIN_PIN 2 // D4
//#define CS_PIN  3 // D9/RX
//#define CLK_PIN 0 // D3

// for NodeMCU 1.0/D1 mini
//#define DIN_PIN 15  // D8
//#define CS_PIN  13  // D7
//#define CLK_PIN 12  // D6

// =======================================================================
// https://en.wikipedia.org/wiki/Summer_Time_in_Europe
// from
// Sunday (31 − ((((5 × y) ÷ 4) + 4) mod 7)) March at 01:00 UTC
// to
// Sunday (31 − ((((5 × y) ÷ 4) + 1) mod 7)) October at 01:00 UTC

//int checkSummerTime()
//{
//  if(month>3 && month<10) return 1;
//  if(month==3 && day>=31-(((5*year/4)+4)%7) ) return 1;
//  if(month==10 && day<31-(((5*year/4)+1)%7) ) return 1;
//  return 0;
//}
// =======================================================================
