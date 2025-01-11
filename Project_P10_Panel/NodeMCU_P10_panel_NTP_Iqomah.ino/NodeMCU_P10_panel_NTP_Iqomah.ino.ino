/*-------------Wemos D1 P10 Panel Clock With RTC Backup-----------------*/
/*---------------Source Code by Bonny Useful @Busel7--------------------*/
/*--------------Modified & Adapted by Habib.m @Hab_mz-------------------*/
/*------------------------------V3.0------------------------------------*/
/*----------------------------------------------------------------------*/

//UPLOAD SKETCH ON LOLIN (WEMOS) D1 MINI(CLONE)

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
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <DMDESP.h>
#include <PrayerTimes.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "SystemFont5x7.h"
#include "Font_6x14.h" 
#include "verdanav2.h"
#include "ElektronMart6x12.h"
#include "A_variables.h"
#include "B_Lib_Buzzer.h"
#include "C_Functions.h"
#include "D_WebSetup.h"

//------------------------------------------------------------------------------------------------------//
//------------------------------------------------------Void Setup--------------------------------------//
void setup() {
  DEBUG_BEGIN(115200);
  pinMode(buzzer, OUTPUT);
  pinMode(tombol_jam, INPUT);

  EEPROM.begin(512);
//  reload_EEPROM();
  
  if (analogRead(tombol_jam) > 180) {
    norMode = false;
  }

  if(norMode){
    //------------------------------------------------------WiFi Connect------------------------------------//
    DEBUG_PRINT("Connecting WiFi ");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      setCnt = !setCnt;
      DEBUG_PRINT(".");
      if(setCnt){ tone(buzzer,685); }
      else{ noTone(buzzer); }
      delay(500);}
    noTone(buzzer);
    DEBUG_PRINTLN("");
    DEBUG_PRINT("Connected: "); DEBUG_PRINTLN(WiFi.localIP());
    
    timeClient.begin();
    timeClient.setTimeOffset(25200); //3600 x gmt+7
    setSyncProvider(requestSync);
    Disp.start();
  //-----------------------------------------------------NTP Clock Setup----------------------------------//
    DEBUG_PRINTLN("Getting data ...");
    getTime();
    DEBUG_PRINTLN("Data loaded");
    
    delay(2000);
    WiFi.disconnect();
    DEBUG_PRINTLN("wifi Disconnect Success");
    HitungJadwalSholat();
//    if (_hour24 == 0 || _hour24 == 00) {
//    tone(buzzer,300); delay(500); noTone(buzzer);
//      DEBUG_PRINTLN(" Date Failed, Resetting...");
//      ESP.restart();
//    }
  }
  else{
    tone(buzzer,300); delay(500); noTone(buzzer);
    WiFi.softAP("JWS SETUP", "12345678");
    IPAddress myIP = WiFi.softAPIP();
    server.on("/", handleRoot);
    server.on("/overwriteSettings",overwriteSettings);
    httpUpdater.setup(&server);
    server.begin();
//    reload_EEPROM();  
  }
}

//-----------------------------------------------------------------------------------------------------//
//------------------------------------------------------Void Loop--------------------------------------//
void loop() {
  if(norMode){
    Disp.loop();
    GetDateTime();
    AlarmSholat();
    tampilinfo();
    brightSet();
  }
  else server.handleClient();
}
 
