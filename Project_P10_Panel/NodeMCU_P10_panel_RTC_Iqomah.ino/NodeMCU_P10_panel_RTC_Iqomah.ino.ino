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

#include <Wire.h>
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);

#include <DMDESP.h>
#include <PrayerTimes.h>

#include "SystemFont5x7.h"
#include "Font_6x14.h" 
#include "verdanav2.h"
#include "ElektronMart6x12.h"

#include "A_variables.h"
#include "B_RTCSetup.h"
#include "C_WebSetup.h"
#include "D_Lib_Buzzer.h"
#include "E_Functions.h"

//------------------------------------------------------------------------------------------------------//
//------------------------------------------------------Void Setup--------------------------------------//
void setup() {
  DEBUG_BEGIN(115200);
  pinMode(buzzer, OUTPUT);

  EEPROM.begin(512);
//  reload_EEPROM();
  
  mulaiRTC();
  wifiConnect();
  BacaRTC();
  
  server.on("/", handleRoot);
  server.on("/overwriteSettings",overwriteSettings);
  httpUpdater.setup(&server);
  server.begin();

  Disp.start();
  UpdateWaktu();
//  reload_EEPROM();  
}

//-----------------------------------------------------------------------------------------------------//
//------------------------------------------------------Void Loop--------------------------------------//
void loop() {
  server.handleClient();
  Disp.loop();
  UpdateWaktu();
  AlarmSholat();
  tampilinfo();
  brightSet();
}
 
