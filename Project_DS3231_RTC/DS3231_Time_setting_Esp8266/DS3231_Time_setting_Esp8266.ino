/*----------------DS3231_TimeSetting-------------------*/
/*-------------Source Code by---------------------------*/
/*------Modified & Adapted by Habib.m @Hab_mz-----------*/
/*-----------------------V1.0---------------------------*/

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);

#include "rtc.h"

void setup(){
  Serial.begin(115200);

  mulaiRTC();
  
//  Rtc.SetDateTime(RtcDateTime(2025,    1,    2,   14,  5,     30));
//                              Tahun, bulan, tgl, jam, menit, detik
}

void loop() {
  RtcDateTime now = Rtc.GetDateTime();
  RtcTemperature temp = Rtc.GetTemperature();
  float celsius = temp.AsFloatDegC();
  uint8_t rJam = now.Hour();
  uint8_t rMen = now.Minute();
  uint8_t rDet = now.Second();
  uint8_t rTgl = now.Day();
  uint8_t rHar = now.DayOfWeek();
  uint8_t rBul = now.Month();
  unsigned int rTah = now.Year();
  
   Serial.print("jam= ");
   Serial.print(String(rJam));
   Serial.print(":");
   Serial.print(String(rMen));
   Serial.print(":");
   Serial.println(String(rDet));
   
   Serial.print("  tgl= ");
   Serial.print(String(rHar));
   Serial.print(", ");
   Serial.print(String(rTgl));
   Serial.print("-");
   Serial.print(String(rBul));
   Serial.print("-");
   Serial.println(String(rTah));
   
   Serial.print("  Suhu= ");
   Serial.print(String(celsius));

   delay(1000);
}
