/*-------------Wemos D1 P10 Panel Clock With RTC Backup-----------------*/
/*---------------Source Code by Bonny Useful @Busel7--------------------*/
/*--------------Modified & Adapted by Habib.m @Hab_mz-------------------*/
/*------------------------------V3.0------------------------------------*/
/*----------------------------------------------------------------------*/

//UPLOAD SKETCH ON LOLIN (WEMOS) D1 MINI(CLONE)

// Define DEBUG to enable debugging; comment it out to disable
//#define DEBUG

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
  reload_EEPROM();

  //------------------------------------------------------WiFi Connect------------------------------------//
  DEBUG_PRINT("Connecting WiFi...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
        setCnt = !setCnt;
        DEBUG_PRINT(".");
        if(setCnt){ digitalWrite(buzzer,HIGH); }
        else{ digitalWrite(buzzer,LOW); }
        delay(500);
  }
  digitalWrite(buzzer,LOW);
  DEBUG_PRINTLN("");
  DEBUG_PRINT("Connected: "); DEBUG_PRINTLN(WiFi.localIP());
  ipAdress = WiFi.localIP().toString();
    
  timeClient.begin();
  timeClient.setTimeOffset(25200); //3600 x gmt+7
  setSyncProvider(requestSync);
//-----------------------------------------------------NTP Clock Setup----------------------------------//
  DEBUG_PRINTLN("Getting data ...");
  getTime();
  DEBUG_PRINTLN("Data loaded");
    
  if (Epoch_Time < 1357041600) {
    digitalWrite(buzzer,HIGH); delay(500); digitalWrite(buzzer,LOW);
    DEBUG_PRINTLN(" Date Failed, Resetting...");
    ESP.restart();
  }
  
  server.on("/", handleRoot);
  server.on("/overwriteSettings",overwriteSettings);
  httpUpdater.setup(&server);
  server.begin();
  Disp.start();
  HitungJadwalSholat();
//  reload_EEPROM();
}

//-----------------------------------------------------------------------------------------------------//
//------------------------------------------------------Void Loop--------------------------------------//
void loop() {
  Disp.loop();
  server.handleClient();
  GetDateTime();
  AlarmSholat();
  tampilinfo();
  brightSet();
  Wifimati1Jam();
}
 
