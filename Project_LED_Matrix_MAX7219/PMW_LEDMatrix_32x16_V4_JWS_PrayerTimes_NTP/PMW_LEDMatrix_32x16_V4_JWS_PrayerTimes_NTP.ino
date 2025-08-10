/*-------------NODEMCU max7219 Panel Clock With NTP Clock---------------*/
/*---------------NODEMCU - 32x16Px MAX7219 LED Matrix-------------------*/
/*-----------------Source Code by Pawel A. Hernik-----------------------*/
/*--------------Modified & Adapted by Habib.m @Hab_mz-------------------*/
/*------------------------------V4.0------------------------------------*/
/*----------------------------------------------------------------------*/

// Define DEBUG to enable debugging; comment it out to disable
#define DEBUG

#ifdef DEBUG
  #define PRINT(...) Serial.print(__VA_ARGS__)
  #define PRINTLN(...) Serial.println(__VA_ARGS__)
  #define BEGIN(baud) Serial.begin(baud)
#else
  #define PRINT(...)    // Do nothing
  #define PRINTLN(...)  // Do nothing
  #define BEGIN(baud)   // Do nothing
#endif

//------------------------------------------------------Additional Lib----------------------------------//
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "globalVars.h"
#include "prayerCalc.h"
#include "DHTSensor.h"
#include "fonts.h"
#include "max7219.h"
#include "timeCalc.h"


void setup(){
  BEGIN(115200);
  dht.begin();
  initMAX7219();
  
  sendCmdAll(CMD_SHUTDOWN, 1);
  sendCmdAll(CMD_INTENSITY, 0);
  
  PRINT("Connecting to WiFi");
  WiFi.begin(ssid, password);
  clr();
  xPos=0;
  printString("CONNECT..", font3x7);
  refreshAll();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    PRINT(".");
  }
  
  PRINTLN(""); PRINT("CONNECTED TO: "); PRINTLN(WiFi.localIP());
  
  clr();
  xPos=0;
  printString((WiFi.localIP().toString()).c_str(), font3x7);
  
  timeClient.begin();
  timeClient.setTimeOffset(3600 * utcOffset); //3600 x gmt+7
  setSyncProvider(requestSync);
  getTime();
  getTempHum();
  printString(" - OK ", font3x7);
  
  delay(1000);
  WiFi.disconnect();
  Serial.println("wifi Disconnect Success");
  refreshAll();
}

// =======================================================================

void loop(){
  if(millis()-updTime>30000) { // update time every 600s=10m
    updTime = millis();
    getTempHum();
    JadwalSholat();  
  }
  dots = (millis() % 1000) < 500;     // blink 2 times/sec
  switch(tmpUtama) {
    case 0: jamBesar(); break;
    case 1: jamKecilTanggal(); break;
    case 2: jamBesarDetik(); break;
    case 3: jamKecilDHT(); break;
    case 4: tampilJadwalSholat(); break;
  }
  refreshAll();
  updateTime();
//  delay(10);
}
