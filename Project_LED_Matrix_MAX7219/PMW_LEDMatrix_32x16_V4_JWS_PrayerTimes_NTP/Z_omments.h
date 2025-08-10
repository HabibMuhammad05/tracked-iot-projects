// MAX7219 functions by Pawel A. Hernik
// 2016.12.10 updated for rotated LED Martices, define ROTATE below (0,90 or 270)

// MAX7219 commands:


//char* monthNames[] = {"STY","LUT","MAR","KWI","MAJ","CZE","LIP","SIE","WRZ","PAZ","LIS","GRU"};

// MON, TUE, WED, THU, FRI, SAT, SUN
// JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC
// Thu, 19 Nov 2015
// decodes: day, month(1..12), dayOfWeek(1-Mon,7-Sun), year
//void decodeDate(String date)
//{
//  switch(date.charAt(0)) {
//    case 'M': NTPdayOfWeek=1; break;
//    case 'T': NTPdayOfWeek=(date.charAt(1)=='U')?2:4; break;
//    case 'W': NTPdayOfWeek=3; break;
//    case 'F': NTPdayOfWeek=5; break;
//    case 'S': NTPdayOfWeek=(date.charAt(1)=='A')?6:7; break;
//  }
//  int midx = 6;
//  if(isdigit(date.charAt(midx))) midx++;
//  midx++;
//  switch(date.charAt(midx)) {
//    case 'F': NTPmonth = 2; break;
//    case 'M': NTPmonth = (date.charAt(midx+2)=='R') ? 3 : 5; break;
//    case 'A': NTPmonth = (date.charAt(midx+1)=='P') ? 4 : 8; break;
//    case 'J': NTPmonth = (date.charAt(midx+1)=='A') ? 1 : ((date.charAt(midx+2)=='N') ? 6 : 7); break;
//    case 'S': NTPmonth = 9; break;
//    case 'O': NTPmonth = 10; break;
//    case 'N': NTPmonth = 11; break;
//    case 'D': NTPmonth = 12; break;
//  }
//  NTPday = date.substring(5, midx-1).toInt();
//  NTPyear = date.substring(midx+4, midx+9).toInt();
//  return;
//}

//void getTime()
//{
//  WiFiClient client;
//  PRINTLN("connecting to www.google.com ...");
//  if(!client.connect("www.google.com", 80)) {
//    PRINTLN("connection failed");
//    return;
//  }
//  client.print(String("GET / HTTP/1.1\r\n") +
//               String("Host: www.google.com\r\n") +
//               String("Connection: close\r\n\r\n"));
//
//  int repeatCounter = 10;
//  while (!client.available() && repeatCounter--) {
//    delay(200); PRINTLN("y.");
//  }
//
//  String line;
//  client.setNoDelay(false);
//  int dateFound = 0;
//  while(client.connected() && client.available() && !dateFound) {
//    line = client.readStringUntil('\n');
//    line.toUpperCase();
//    // Date: Thu, 19 Nov 2015 20:25:40 GMT
//    if(line.startsWith("DATE: ")) {
//      localMillisAtUpdate = millis();
//      dateFound = 1;
//      date = line.substring(6, 22);
//      date.toUpperCase();
//      decodeDate(date);
//      //Serial.println(line);
//      h = line.substring(23, 25).toInt();
//      m = line.substring(26, 28).toInt();
//      s = line.substring(29, 31).toInt();
////      summerTime = checkSummerTime();
////      if(h+utcOffset+summerTime>23) {
////        if(++day>31) { day=1; month++; };  // needs better patch
////        if(++dayOfWeek>7) dayOfWeek=1; 
////      }
//      PRINTLN(String(h) + ":" + String(m) + ":" + String(s)+"   Date: "+NTPday+"."+NTPmonth+"."+NTPyear+" ["+NTPdayOfWeek+"] "+utcOffset+"h");
//      localEpoc = h * 60 * 60 + m * 60 + s;
//    }
//  }
//  client.stop();
//}

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


//int charWidth(char c, const uint8_t *font) {
//  int fwd  = pgm_read_byte(font);      // max width (not used here)
//  int fht  = pgm_read_byte(font + 1);
//  int offs = pgm_read_byte(font + 2);
//  int last = pgm_read_byte(font + 3);
//  if (c < offs || c > last) return 0;
//
//  int fht8 = (fht + 7) / 8;
//  int idx = c - offs;
//  const uint8_t *glyphPtr = font + 4;
//
//  // Step through glyphs to reach the right one
//  for (int i = 0; i < idx; i++) {
//    uint8_t w = pgm_read_byte(glyphPtr);
//    glyphPtr += 1 + w * fht8;
//  }
//
//  return pgm_read_byte(glyphPtr); // width of desired character
//}

//int stringWidth(const char *s, const uint8_t *font) {
//  int wd = 0;
//  while (*s) wd += charWidth(*s++, font) + 1;
//  return wd > 0 ? wd - 1 : 0; // remove final spacing
//}
//
//int stringWidth(String str, const uint8_t *font) {
//  return stringWidth(str.c_str(), font);
//}


//void printCentered(const char* txt, const uint8_t* font, int displayW, int displayY = 0, int tweak = 0) {
//  xPos = (displayW - stringWidth(txt, font)) / 2 + tweak;
//  yPos = displayY;
//  printString(txt, font);
//}
//
//void printCentered(String txt, const uint8_t* font, int displayW, int displayY = 0, int tweak = 0) {
//  printCentered(txt.c_str(), font, displayW, displayY, tweak);
//}

//void printCentered(const char* txt, const uint8_t* font, int displayW, int displayY = 0, int tweak = 0) {
//  xPos = (displayW - stringWidth(txt, font)) / 2 + tweak;
//  yPos = displayY;
//  printString(txt, font);
//}
//
//void printCentered(String txt, const uint8_t* font, int displayW, int displayY = 0, int tweak = 0) {
//  printCentered(txt.c_str(), font, displayW, displayY, tweak);
//}


//    sprintf(txt, "%s", namaHari[NTPdayOfWeek - 1]);
//    yPos = 1;
//    xPos = centerTextX(txt, fontBesar, 32);
//    printString(txt, fontBesar);
