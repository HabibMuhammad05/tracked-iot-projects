
void getTime(){ 
  timeClient.update();
  Epoch_Time = timeClient.getEpochTime()+ 1;
  Serial.println(Epoch_Time);
  delay(500);
  if( Epoch_Time >= 1357041600) { 
     setTime(Epoch_Time);
  }
}

void GetDateTime() {
//Uncomment for Debug
//Serial.println( String(_day) + "-" + String(_month) + "-" + String(_year));
//Serial.println( String(_hour24) + ":" + String(_minute) + ":" + String(_second));

  _day=day();
  _month=month();
  _year=year();
  _hour24=hour();
  _minute=minute();
  _second=second();
  _dtw=weekday();

  hr24=_hour24;
  
  if (hr24>12) { _hour12=hr24-12; }
  else if (hr24==0) { _hour12=12; }
  else { _hour12=hr24; }

  if (hr24<12) { st="AM"; }
  else { st="PM"; } 
}

void HitungJadwalSholat() {

  /*
    CALCULATION METHOD
    ------------------
    Jafari,   // Ithna Ashari
    Karachi,  // University of Islamic Sciences, Karachi
    ISNA,     // Islamic Society of North America (ISNA)
    MWL,      // Muslim World League (MWL)
    Makkah,   // Umm al-Qura, Makkah
    Egypt,    // Egyptian General Authority of Survey
    Custom,   // Custom Setting
    JURISTIC
    --------
    Shafii,    // Shafii (standard)
    Hanafi,    // Hanafi
    ADJUSTING METHOD
    ----------------
    None,        // No adjustment
    MidNight,   // middle of night
    OneSeventh, // 1/7th of night
    AngleBased, // angle/60th of night
    TIME IDS
    --------
    Fajr,
    Sunrise,
    Dhuhr,
    Asr,
    Sunset,
    Maghrib,
    Isha
  */
  
  int tahun = year();
  int bulan = month();
  int tanggal = day();
  float latitude = -6.939121;
  float longitude = 106.934425;
  int zonawaktu = 7;
  
  set_calc_method(Karachi);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(AngleBased);
  set_fajr_angle(20);
  set_isha_angle(18);  

  get_prayer_times(tahun, bulan, tanggal, latitude, longitude, zonawaktu, times);
}

void brightSet(){
  if ( (_hour24 >= 0 && _hour24 <= 7) || (_hour24 >=17 && _hour24 <= 23)){ Disp.setBrightness(8); }
  
  else { Disp.setBrightness(15); }
}

void textCenter(int y,String Msg) {
  int center = int((Disp.width()-Disp.textWidth(Msg)) / 2);
  Disp.drawText(center,y,Msg);
}

void JadwalAnim(String atas, String bawah){
  static uint8_t x;
  static uint8_t s; // 0=in, 1=out
  static uint32_t pM;
  uint32_t cM = millis();

  if (cM - pM > 8) {
    if (s == 0 and x < 8) {
      pM = cM;
      x++;
    }
    if (s == 1 and x > 0) {
      pM = cM;
      x--;
    }
  }
  
  if (cM - pM > 1500 and x == 8) {
    s = 1; 
  }
  
  if (x == 0 and s == 1) { 
    Disp.clear();
    ijadwal++;
    s = 0;
  }
  
  textCenter(x -8, atas); 
  textCenter(15 - x, bawah);
}

void TeksJalan(int y, uint8_t kecepatan) {
  static uint32_t pM;
  static uint32_t x;
  int width = Disp.width();
  Disp.setFont(SystemFont5x7); // Tentukan huruf
  int fullScroll = Disp.textWidth(teks) + width;
  if((millis() - pM) > kecepatan) { 
    pM = millis();
    if (x < fullScroll) {
      ++x;
    } else {
      x = 0;
      scrAnim = false;
//      tmpinfo = 2;
//      Clr = true;
      return;
    }
    Disp.drawText(width - x, y, teks);
  }  
}

void TampilJadwalSholat() {
  static uint32_t pM;
  uint32_t cM = millis();
  
  char sholat[7];
  char jam[5];
  char TimeName[][8] = {"SUBUH","TERBIT","DZUHR","ASHAR","TRBNM","MAGRB","ISYA"};
  int hours, minutes;
  int ihti = 2;

  if (cM - pM >= 0) {
    pM = cM;
    Disp.drawRect(0,0,32,15,0,0);
    
//    if (i == 1) {i = 2;} // Abaikan Terbit
    if (ijadwal == 4) {ijadwal = 5;} // Abaikan Terbenam

    get_float_time_parts(times[ijadwal], hours, minutes);
    minutes = minutes + ihti;

    if (minutes >= 60) {
      minutes = minutes - 60;
      hours ++;
    }

    char *sholat = TimeName[ijadwal];
    sprintf(jam,"%02d:%02d", hours, minutes);     
    Disp.setFont(verdanav2);
    JadwalAnim(sholat, jam);
//    textCenter(-1, sholat); 
//    textCenter(7, jam);
     
//    ijadwal++;

    if (ijadwal > 6) {
      get_float_time_parts(times[0], hours, minutes);
      minutes = minutes + ihti;
      if (minutes < 11) {
        minutes = 60 - minutes;
        hours --;
      } else {
        minutes = minutes - 10 ;
      }
      sprintf(jam,"%02d:%02d", hours, minutes);
      Disp.drawRect(0,0,32,15,0,0);
      Disp.setFont(verdanav2);
      JadwalAnim("IMSAK", jam);
//      textCenter(-1, "IMSAK"); 
//      textCenter(7, jam); 
      
      if (ijadwal > 7) {
        ijadwal = 0;
        Disp.drawRect(0,0,32,15,0,0);
        tmpinfo = 0;
      }
    }   
  }
}

void scrolling_date() {  
  static uint8_t xs;
  static uint8_t s; // 0=in, 1=out
  static uint32_t pM;
  uint32_t cM = millis();
  teks = String(String(nameoftheday[_dtw-1]) + ", " +String(_day) + "-" + String(month_name[_month-1]) + "-" + String(_year) + " ");
  
  
  unsigned long waktusekarang = millis();
  if(Clr){
      Clr = false;
      HitungJadwalSholat();
      Disp.drawRect(0,0,32,15,0,0);
  }

  if (cM - pM > 8) {
    if (s == 0 and xs < 6) {
      pM = cM;
      xs++;
    }
    if (s == 1 and xs > 0) {
      pM = cM;
      xs--;
    }
  }
  
  if (cM - pM > 0 and xs == 6 and !scrAnim) {
    TeksJalan(8, 60); // Tampilkan teks berjalan TeksJalan(posisi y, kecepatan);
    s = 1; 
  }
  
  if (xs == 0 and s == 1) { 
    Disp.clear();
    tmpinfo = 2;
    Clr = true;
    scrAnim = true;
    s = 0;
  }
  
  if(waktusekarang-dotMillis>= dotInt){ dotBool = !dotBool; dotMillis=millis(); }
  
  Disp.setFont(verdanav2); // Tentukan huruf
  Disp.drawText(1, xs - 7, (_hour24 < 10 ? "0" : "") + String(_hour24));
  textCenter(-1, (dotBool ? ":" : " "));
  Disp.drawText(18, xs - 7, (_minute < 10 ? "0" : "") + String(_minute));
  
  TeksJalan(8, 60); // Tampilkan teks berjalan TeksJalan(posisi y, kecepatan);
  
}

void BigClock(){
    static uint8_t x;
    static uint8_t s; // 0=in, 1=out
    static uint32_t pM;
    uint32_t cM = millis();
    
    
    if (cM - pM > 20) {
      if (s == 0 and x < 12) {
        pM = cM;
        x++;
      }
      
      if (_second == 25 and x == 12) {
        s = 1;    
      }
      
      if (s == 1 and x > 0) {
        pM = cM;
        x--;
      }

      if (x == 0 and s == 1) {
        Disp.clear();
        s = 0;
        tmpinfo = 1;
      }

      Disp.setFont(Font_6x14);
      Disp.drawText(x - 11, 0, (_hour24 < 10 ? "0" : "") + String(_hour24));
     
      Disp.setFont(SystemFont5x7);
      Disp.drawText(Disp.width() - x, 0, (_minute < 10 ? "0" : "") + String(_minute));
      Disp.drawText(Disp.width() - x, 9, (_second <10 ? "0" : "") + String(_second));
    } 
    
//    if(_second == 25){ tmpinfo = 1; }
    
    if (_second %2 == 0) {    
        Disp.drawRect(16,3,17,4, 1,1);
        Disp.drawRect(16,11,17,12, 1,1); 
      }
     else {
        Disp.drawRect(16,3,17,4, 0,0);
        Disp.drawRect(16,11,17,12, 0,0); 
      }
}

void AlarmSholat() {  
  int hours, minutes, seconds;

//---------------------------------------Imsak----------------------------------------------//
  get_float_time_parts(times[0], hours, minutes);
  minutes = minutes + ihti;
  if (minutes < 10) {
    minutes = 60 - minutes;
    hours --;
  } else {
    minutes = minutes - 10 ;
  }
  if (_hour24 == hours && _minute == minutes) {
    tipealarm = 0;
    detikbeep = 0;
    Disp.clear();
    tmpinfo = 3;
  }

//---------------------------------------Subuh----------------------------------------------//
  get_float_time_parts(times[0], hours, minutes);
  minutes = minutes + ihti;
  if (minutes >= 60) {
    minutes = minutes - 60;
    hours ++;
  }
  if (_hour24 == hours && _minute == minutes) {
    tipealarm = 1;
    detikbeep = 0;
    Disp.clear();
    tmpinfo = 3;
  }

//---------------------------------------Dzuhur----------------------------------------------//
  get_float_time_parts(times[2], hours, minutes);
  minutes = minutes + ihti;
  if (minutes >= 60) {
    minutes = minutes - 60;
    hours ++;
  }
  if (_hour24 == hours && _minute == minutes &&  _second == 0 && _dtw != 5) {
    tipealarm = 2;
    detikbeep = 0;
    Disp.clear();
    tmpinfo = 3;
  } 
  else if (_hour24 == hours && _minute == minutes && _second == 0 && _dtw == 5) {
    tipealarm = 3;
    detikbeep = 0;
    Disp.clear();
    tmpinfo = 3;
  }

//---------------------------------------Ashar----------------------------------------------//
  get_float_time_parts(times[3], hours, minutes);
  minutes = minutes + ihti;
  if (minutes >= 60) {
    minutes = minutes - 60;
    hours ++;
  }
  if (_hour24 == hours && _minute == minutes && _second == 0) {
    tipealarm = 4;
    detikbeep = 0;
    Disp.clear();
    tmpinfo = 3;
  }
  
//--------------------------------------Maghrib----------------------------------------------//
  get_float_time_parts(times[5], hours, minutes);
  minutes = minutes + ihti;
  if (minutes >= 60) {
    minutes = minutes - 60;
    hours ++;
  }
  if (_hour24 == hours && _minute == minutes && _second == 0) {
    tipealarm = 5;
    detikbeep = 0;
    Disp.clear();
    tmpinfo = 3;
  }

//----------------------------------------Isya-----------------------------------------------//
  get_float_time_parts(times[6], hours, minutes);
  minutes = minutes + ihti;
  if (minutes >= 60) {
    minutes = minutes - 60;
    hours ++;
  }
  if (_hour24 == hours && _minute == minutes && _second == 0) {
    tipealarm = 6;
    detikbeep = 0;
    Disp.clear();
    tmpinfo = 3;
  }  
}

void TampilAdzan() {
  BunyiBeep(2,30);

  static uint32_t pM;
  uint32_t cM = millis();
  static uint16_t det;
  int hours, minutes;
  
  if (beep){
    if (tipealarm == 0) {
    Disp.setFont(verdanav2);
    textCenter(0, "TANBIH");
    Disp.setFont(SystemFont5x7);
    textCenter(8, "IMSAK");
    }
    
    if (tipealarm == 1) {
      Disp.setFont(verdanav2);
      textCenter(0, "ADZAN");
      Disp.setFont(SystemFont5x7);
      textCenter(8, "SUBUH");
    }
    
    if (tipealarm == 2) {
      Disp.setFont(verdanav2);
      textCenter(0, "ADZAN");
      Disp.setFont(SystemFont5x7);
      textCenter(8, "DZUHR");
    }
    
    if (tipealarm == 3) {
      Disp.setFont(verdanav2);
      textCenter(0, "ADZAN");
      Disp.setFont(SystemFont5x7);
      textCenter(8, "JUMAT");
    }
    
    if (tipealarm == 4) {
      Disp.setFont(verdanav2);
      textCenter(0, "ADZAN");
      Disp.setFont(SystemFont5x7);
      textCenter(8, "ASHAR");
    }
    
    if (tipealarm == 5) {
      Disp.setFont(verdanav2);
      textCenter(0, "ADZAN");
      Disp.setFont(SystemFont5x7);
      textCenter(8, "MAGRB");
    } 
    
    if (tipealarm == 6) {
      Disp.setFont(verdanav2);
      textCenter(0, "ADZAN");
      Disp.setFont(SystemFont5x7);
      textCenter(8, "ISYA");
    }  
  }else { Disp.clear(); }
  

  if (cM-pM >= 1000) {
    det++;
    if (tipealarm == 0 or tipealarm == 1 or tipealarm == 2 or tipealarm == 3 or tipealarm == 4 or tipealarm == 5 or tipealarm == 6) {
      if (det == 30) {
        Disp.clear();
        det = 0;
        detikbeep = 0;
        tmpinfo = 0;
      } 
    }
    pM = cM;
  }  
}

void tampilinfo() {
  switch(tmpinfo) {
    case 0 :
      Disp.drawRect(0,0,32,15,0,0);
      noTone(buzzer);
      BigClock();
      break;

    case 1 :
      scrolling_date();
      break;

    case 2 :
      TampilJadwalSholat();
      break;

    case 3 :
      TampilAdzan();
  }
}
