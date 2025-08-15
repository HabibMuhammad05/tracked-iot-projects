void Wifimati1Jam() {
  static bool cekWifi = false;

  if (!cekWifi && millis() >= 3600000) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);   
    cekWifi = true;  
    DEBUG_PRINTLN("WiFi off after 1 hour.");
  }
}

// I2C_ClearBus menghindari gagal baca RTC (nilai 00 atau 165)

RtcDateTime now;
int celsius;

int I2C_ClearBus() {
  
  #if defined(TWCR) && defined(TWEN)
    TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
  #endif

  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  delay(2500);  // Wait 2.5 secs. This is strictly only necessary on the first power
  // up of the DS3231 module to allow it to initialize properly,
  // but is also assists in reliable programming of FioV3 boards as it gives the
  // IDE a chance to start uploaded the program
  // before existing sketch confuses the IDE by sending Serial data.

  boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master. 
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
    clockCount--;
  // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(SCL, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(SCL, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(SCL, INPUT); // release SCL LOW
    pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(SDA, INPUT); // remove pullup.
  pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(SDA, INPUT); // remove output low
  pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(SCL, INPUT);
  return 0; // all ok
  
}



// MULAI RTC DS3231

void mulaiRTC() {

  int rtn = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
  if (rtn != 0) {
    DEBUG_PRINTLN(F("I2C bus error. Could not clear"));
    if (rtn == 1) {
      DEBUG_PRINTLN(F("SCL clock line held low"));
    } else if (rtn == 2) {
      DEBUG_PRINTLN(F("SCL clock line held low by slave clock stretch"));
    } else if (rtn == 3) {
      DEBUG_PRINTLN(F("SDA data line held low"));
    }
  } else { // bus clear, re-enable Wire, now can start Wire Arduino master
    Wire.begin();
  }
  
  Rtc.Begin();

  if (!Rtc.GetIsRunning()) {
    
    Rtc.SetIsRunning(true);
    
  }
  
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  
  DEBUG_PRINTLN("Setup RTC selesai");
  
}

void BacaRTC() {
  RtcDateTime now = Rtc.GetDateTime();
  RtcTemperature temp = Rtc.GetTemperature();
  celsius = temp.AsFloatDegC();
  _hour24 = now.Hour();
  _minute = now.Minute();
  _second = now.Second();
  _day = now.Day();
  _dtw = now.DayOfWeek();
  _month = now.Month();
  _year = now.Year();
  
  hr24=_hour24;
}

void updateJam() {
  static unsigned long pM;
  static byte d;

  if (millis() - pM >= 1000) {
    d++;
    if (d < 2) {
      BacaRTC();
//      HitungJadwalSholat();
    } else if (d < 60) {
      _second++;
      if (_second > 59) {
        _second = 0;
        _minute++;
        if (_minute > 59) {
          _minute = 0;
          _hour24++;
          if (_hour24 > 23) _hour24 = 0;
          hr24 = _hour24;
        }
      }
    } else {
      d = 0;
    }
    pM = millis();
  }

  _hour12 = (hr24 == 0) ? 12 : (hr24 > 12 ? hr24 - 12 : hr24);
  st = (hr24 < 12) ? "AM" : "PM";
  
  unsigned long printMillis = millis();
  if (printMillis - previousPrint >= 2000) {
    previousPrint = printMillis;
    DEBUG_PRINT("Current time: ");
    DEBUG_PRINT( String(_hour24) + ":" + String(_minute) + ":" + String(_second) + "  Date:");
    DEBUG_PRINTLN( String(_dtw) + ", " + String(_day) + "-" + String(_month) + "-" + String(_year));
  }
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
    Isha  -7.061398250560459, 110.44408790210662
  */
  
  int tahun = _year;
  int bulan = _month;
  int tanggal = _day;
  float latitude = lati;
  float longitude = longi;
  int zonawaktu = 7;
  
  set_calc_method(Karachi);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(AngleBased);
  set_fajr_angle(20);
  set_isha_angle(18);  

  get_prayer_times(tahun, bulan, tanggal, latitude, longitude, zonawaktu, times);
}

void brightSet(){
  if (_hour24 >= 0 && _hour24 <= 6){ Disp.setBrightness(3); }
  else if (_hour24 >= 18 && _hour24 <= 23){ Disp.setBrightness(6); }
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
//  Disp.setFont(SystemFont5x7); // Tentukan huruf
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
  if(mod){
    teks = String(PesanTeks + "    " + String(nameoftheday[_dtw-1]) + ", " +String(_day) + "-" + String(month_name[_month-1]) + "-" + String(_year) + " ");
  }else{
    teks = String(PesanTeks + "    WEB: " + ipAdress + " ");
  }
 
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
    Disp.setFont(verdanav2); // Tentukan huruf
    TeksJalan(8, 50); // Tampilkan teks berjalan TeksJalan(posisi y, kecepatan);
    s = 1; 
  }
  
  if (xs == 0 and s == 1) { 
    Disp.clear();
    mod = !mod;
    tmpinfo = 0;
    Clr = true;
    scrAnim = true;
    s = 0;
  }
  
  if(waktusekarang-dotMillis>= dotInt){ dotBool = !dotBool; dotMillis=millis(); }
  
  Disp.setFont(verdanav2); // Tentukan huruf
  Disp.drawText(1, xs - 7, (_hour24 < 10 ? "0" : "") + String(_hour24));
  textCenter(-1, (dotBool ? ":" : " "));
  Disp.drawText(18, xs - 7, (_minute < 10 ? "0" : "") + String(_minute));
  
  Disp.setFont(verdanav2); // Tentukan huruf
  TeksJalan(8, 50); // Tampilkan teks berjalan TeksJalan(posisi y, kecepatan);
  
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
        tmpinfo = (_minute % 2 == 0) ? 1 : 2;
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
  if (_hour24 == hours && _minute == minutes &&  _second == 0 && _dtw != 6) {
    tipealarm = 2;
    detikbeep = 0;
    Disp.clear();
    tmpinfo = 3;
  } 
  else if (_hour24 == hours && _minute == minutes && _second == 0 && _dtw == 6) {
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
      menitiqmh = iqmhs;
    }
    
    if (tipealarm == 2) {
      Disp.setFont(verdanav2);
      textCenter(0, "ADZAN");
      Disp.setFont(SystemFont5x7);
      textCenter(8, "DZUHR");
      menitiqmh = iqmhd;
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
      menitiqmh = iqmha;
    }
    
    if (tipealarm == 5) {
      Disp.setFont(verdanav2);
      textCenter(0, "ADZAN");
      Disp.setFont(SystemFont5x7);
      textCenter(8, "MAGRB");
      menitiqmh = iqmhm;
    } 
    
    if (tipealarm == 6) {
      Disp.setFont(verdanav2);
      textCenter(0, "ADZAN");
      Disp.setFont(SystemFont5x7);
      textCenter(8, "ISYA");
      menitiqmh = iqmhi;
    }  
  }else { Disp.clear(); }
  
  
  if (cM-pM >= 1000) {
    det++; 
    if (det >= 30){
      if (tipealarm == 0 or tipealarm == 3) {
        BunyiBeep(0,0);
        Disp.clear();
        det = 0;
        detikbeep = 0;
        tmpinfo = 0;
      }
      if (tipealarm == 1 or tipealarm == 2 or tipealarm == 4 or tipealarm == 5 or tipealarm == 6) {
        BunyiBeep(0,0);
        Disp.clear();
        det = 0;
        detikbeep = 0;
        tmpinfo = 4;
      }
    }
    pM = cM;
  }  
}

void Iqomah() {
  static uint32_t pM;
  uint32_t cM = millis(); 
  teks = "TUNGGU IQOMAH ";

  Disp.setFont(verdanav2);
  TeksJalan(-1, 40); 
  
  Disp.setFont(SystemFont5x7);   
  Disp.drawText(1, 9, (menitiqmh < 10 ? "0" : "") + String(menitiqmh));
  Disp.drawText(20, 9, (detikiqmh < 10 ? "0" : "") + String(detikiqmh));
  Disp.drawText(14, 9, (detikiqmh % 2 ? ":" : " "));
  
  if (detikiqmh == 60) {
    detikiqmh = 0;
  }
  if (cM - pM >= 1000) {
    pM = cM;
    detikiqmh--;
    if (menitiqmh == 0 && detikiqmh == 0) {
      Disp.clear();
      detikbeep = 0;
      detikiqmh = 59;
      tmpinfo = 5;
    }
    if (detikiqmh < 0) {
      detikiqmh = 59;
      menitiqmh--;
    }
  }
}

void TampilSaatSholat() {
  static uint32_t pM;
  uint32_t cM = millis();
  static uint32_t det;
  teks = "LURUSKAN DAN RAPATKAN SHAF - MOHON MATIKAN/BISUKAN HANDPHONE ";
  
  BunyiBeep(1,10);
  Disp.setFont(ElektronMart6x12);
  TeksJalan(2, 80);
  
  if (cM - pM >= 1000) {
    pM = cM;
    det++;

    if (det == durasiSholat * 60) {
      Disp.clear();
      detikbeep = 0;
      det = 0;
      tmpinfo = 0;
    }
  }  
}

void tampilinfo() {
  switch(tmpinfo) {
    case 0 :
      Disp.drawRect(0,0,32,15,0,0);
      digitalWrite(buzzer,LOW);
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
      break;
      
    case 4 :
      Iqomah();
      break;
      
    case 5 :
      TampilSaatSholat();
      break;
  }
}
