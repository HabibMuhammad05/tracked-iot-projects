
// PENGHITUNG JADWAL SHOLAT

void JadwalSholat() {

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

  float latitude = -6.16;
  float longitude = 106.61;
  int zonawaktu = 7;
  
  set_calc_method(Karachi);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(AngleBased);
  set_fajr_angle(20);
  set_isha_angle(18);

  get_prayer_times(year, month, day, latitude, longitude, zonawaktu, times);

}

//char* monthNames[] = {"STY","LUT","MAR","KWI","MAJ","CZE","LIP","SIE","WRZ","PAZ","LIS","GRU"};

// MON, TUE, WED, THU, FRI, SAT, SUN
// JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC
// Thu, 19 Nov 2015
// decodes: day, month(1..12), dayOfWeek(1-Mon,7-Sun), year
void decodeDate(String date)
{
  switch(date.charAt(0)) {
    case 'M': dayOfWeek=1; break;
    case 'T': dayOfWeek=(date.charAt(1)=='U')?2:4; break;
    case 'W': dayOfWeek=3; break;
    case 'F': dayOfWeek=5; break;
    case 'S': dayOfWeek=(date.charAt(1)=='A')?6:7; break;
  }
  int midx = 6;
  if(isdigit(date.charAt(midx))) midx++;
  midx++;
  switch(date.charAt(midx)) {
    case 'F': month = 2; break;
    case 'M': month = (date.charAt(midx+2)=='R') ? 3 : 5; break;
    case 'A': month = (date.charAt(midx+1)=='P') ? 4 : 8; break;
    case 'J': month = (date.charAt(midx+1)=='A') ? 1 : ((date.charAt(midx+2)=='N') ? 6 : 7); break;
    case 'S': month = 9; break;
    case 'O': month = 10; break;
    case 'N': month = 11; break;
    case 'D': month = 12; break;
  }
  day = date.substring(5, midx-1).toInt();
  year = date.substring(midx+4, midx+9).toInt();
  return;
}

void getTime()
{
  WiFiClient client;
  DEBUG(Serial.print("connecting to www.google.com ...");)
  if(!client.connect("www.google.com", 80)) {
    DEBUG(Serial.println("connection failed");)
    return;
  }
  client.print(String("GET / HTTP/1.1\r\n") +
               String("Host: www.google.com\r\n") +
               String("Connection: close\r\n\r\n"));

  int repeatCounter = 10;
  while (!client.available() && repeatCounter--) {
    delay(200); DEBUG(Serial.println("y."));
  }

  String line;
  client.setNoDelay(false);
  int dateFound = 0;
  while(client.connected() && client.available() && !dateFound) {
    line = client.readStringUntil('\n');
    line.toUpperCase();
    // Date: Thu, 19 Nov 2015 20:25:40 GMT
    if(line.startsWith("DATE: ")) {
      localMillisAtUpdate = millis();
      dateFound = 1;
      date = line.substring(6, 22);
      date.toUpperCase();
      decodeDate(date);
      //Serial.println(line);
      h = line.substring(23, 25).toInt();
      m = line.substring(26, 28).toInt();
      s = line.substring(29, 31).toInt();
//      summerTime = checkSummerTime();
//      if(h+utcOffset+summerTime>23) {
//        if(++day>31) { day=1; month++; };  // needs better patch
//        if(++dayOfWeek>7) dayOfWeek=1; 
//      }
      DEBUG(Serial.println(String(h) + ":" + String(m) + ":" + String(s)+"   Date: "+day+"."+month+"."+year+" ["+dayOfWeek+"] "+utcOffset+"h");)
      localEpoc = h * 60 * 60 + m * 60 + s;
    }
  }
  client.stop();
}

// =======================================================================


void updateTime()
{
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
  long epoch = (curEpoch + 3600 * utcOffset + 86400L) % 86400L;

  h = ((epoch  % 86400L) / 3600) % 24;
  m = (epoch % 3600) / 60;
  s = epoch % 60;
  JadwalSholat();
}
