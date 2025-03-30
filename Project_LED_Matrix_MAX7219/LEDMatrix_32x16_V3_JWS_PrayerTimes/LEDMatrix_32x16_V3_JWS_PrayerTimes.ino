/*------------NODEMCU max7219 Panel Clock With RTC Backup---------------*/
/*-----------------Source Code by Pawel A. Hernik-----------------------*/
/*--------------Modified & Adapted by Habib.m @Hab_mz-------------------*/
/*------------------------------V3.0------------------------------------*/
/*----------------------------------------------------------------------*/

// Define DEBUG to enable debugging; comment it out to disable
#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define DEBUG_BEGIN(baud) Serial.begin(baud)
#else
  #define DEBUG_PRINT(...)    // Do nothing
  #define DEBUG_PRINTLN(...)  // Do nothing
  #define DEBUG_BEGIN(baud)   // Do nothing
#endif

//------------------------------------------------------Additional Lib----------------------------------//
#include <ESP8266WiFi.h>

#include "globalVars.h"
#include "prayerCalc.h"
#include "DHTSensor.h"
#include "fonts.h"
#include "max7219.h"
#include "timeCalc.h"


void setup(){
  buf.reserve(500);
  DEBUG_BEGIN(115200);
  dht.begin();
  initMAX7219();
  
  sendCmdAll(CMD_SHUTDOWN, 1);
  sendCmdAll(CMD_INTENSITY, 0);
  
  DEBUG_PRINT("Connecting to WiFi");
  WiFi.begin(ssid, password);
  clr();
  xPos=0;
  printString("CONNECT..", font3x7);
  refreshAll();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    DEBUG_PRINT(".");
  }
  
  clr();
  xPos=0;
  DEBUG_PRINTLN(""); DEBUG_PRINT("CONNECTED TO: "); DEBUG_PRINTLN(WiFi.localIP());
  printString((WiFi.localIP().toString()).c_str(), font3x7);
  refreshAll();
  
  getTime();
  getTempHum();
}

// =======================================================================

void loop(){
  if(millis()-updTime>600000) { // update time every 600s=10m
    updTime = millis();
    getTime();
    getTempHum();  
  }
  dots = (millis() % 1000) < 500;     // blink 2 times/sec
//  tmpUtama = (millis() / 20000) % 5;  
  switch(tmpUtama) {
    case 0: jamBesar(); break;
    case 1: jamKecilTanggal(); break;
    case 2: jamBesarDetik(); break;
    case 3: jamKecilDHT(); break;
    case 4: tampilJadwalSholat(); break;
  }
  refreshAll();
  updateTime();
  delay(10);
}
