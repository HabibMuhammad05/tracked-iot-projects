// MAX7219 functions by Pawel A. Hernik
// 2016.12.10 updated for rotated LED Martices, define ROTATE below (0,90 or 270)

int xPos=0, yPos=0;
int clockOnly = 0;
unsigned int curTime,updTime=0;
int dots,mode;
// MAX7219 commands:
#define CMD_NOOP   0
#define CMD_DIGIT0 1
#define CMD_DIGIT1 2
#define CMD_DIGIT2 3
#define CMD_DIGIT3 4
#define CMD_DIGIT4 5
#define CMD_DIGIT5 6
#define CMD_DIGIT6 7
#define CMD_DIGIT7 8
#define CMD_DECODEMODE  9
#define CMD_INTENSITY   10
#define CMD_SCANLIMIT   11
#define CMD_SHUTDOWN    12
#define CMD_DISPLAYTEST 15

byte scr[NUM_MAX*8 + 8]; // +8 for scrolled char

void sendCmd(int addr, byte cmd, byte data)
{
  digitalWrite(CS_PIN, LOW);
  for (int i = NUM_MAX-1; i>=0; i--) {
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, i==addr ? cmd : 0);
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, i==addr ? data : 0);
  }
  digitalWrite(CS_PIN, HIGH);
}

void sendCmdAll(byte cmd, byte data)
{
  digitalWrite(CS_PIN, LOW);
  for (int i = NUM_MAX-1; i>=0; i--) {
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, cmd);
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, data);
  }
  digitalWrite(CS_PIN, HIGH);
}

void refresh(int addr) {
  for (int i = 0; i < 8; i++)
    sendCmd(addr, i + CMD_DIGIT0, scr[addr * 8 + i]);
}

void refreshAllRot270() {
  byte mask = 0x01;
  for (int c = 0; c < 8; c++) {
    digitalWrite(CS_PIN, LOW);
    for(int i=NUM_MAX-1; i>=0; i--) {
      byte bt = 0;
      for(int b=0; b<8; b++) {
        bt<<=1;
        if(scr[i * 8 + b] & mask) bt|=0x01;
      }
      shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, CMD_DIGIT0 + c);
      shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, bt);
    }
    digitalWrite(CS_PIN, HIGH);
    mask<<=1;
  }
}

void refreshAllRot90() {
  byte mask = 0x80;
  for (int c = 0; c < 8; c++) {
    digitalWrite(CS_PIN, LOW);
    for(int i=NUM_MAX-1; i>=0; i--) {
      byte bt = 0;
      for(int b=0; b<8; b++) {
        bt>>=1;
        if(scr[i * 8 + b] & mask) bt|=0x80;
      }
      shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, CMD_DIGIT0 + c);
      shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, bt);
    }
    digitalWrite(CS_PIN, HIGH);
    mask>>=1;
  }
}

void refreshAll() {
#if ROTATE==270
  refreshAllRot270();
#elif ROTATE==90
  refreshAllRot90();
#else
  for (int c = 0; c < 8; c++) {
    digitalWrite(CS_PIN, LOW);
    for(int i=NUM_MAX-1; i>=0; i--) {
      shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, CMD_DIGIT0 + c);
      shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, scr[i * 8 + c]);
    }
    digitalWrite(CS_PIN, HIGH);
  }
#endif
}


void clr()
{
  for (int i = 0; i < NUM_MAX*8; i++) scr[i] = 0;
}

void scrollLeft()
{
  for(int i=0; i < NUM_MAX*8+7; i++) scr[i] = scr[i+1];
}

void invert()
{
  for (int i = 0; i < NUM_MAX*8; i++) scr[i] = ~scr[i];
}

void initMAX7219()
{
  pinMode(DIN_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  sendCmdAll(CMD_DISPLAYTEST, 0);
  sendCmdAll(CMD_SCANLIMIT, 7);
  sendCmdAll(CMD_DECODEMODE, 0);
  sendCmdAll(CMD_INTENSITY, 0); // minimum brightness
  sendCmdAll(CMD_SHUTDOWN, 0);
  clr();
  refreshAll();
}
// =======================================================================

int charWidth(char c, const uint8_t *font)
{
  int fwd = pgm_read_byte(font);
  int fht = pgm_read_byte(font+1);
  int offs = pgm_read_byte(font+2);
  int last = pgm_read_byte(font+3);
  if(c<offs || c>last) return 0;
  c -= offs;
  int len = pgm_read_byte(font+4);
  return pgm_read_byte(font + 5 + c * len);
}

// =======================================================================

int stringWidth(const char *s, const uint8_t *font)
{
  int wd=0;
  while(*s) wd += 1+charWidth(*s++, font);
  return wd-1;
}

// =======================================================================

int stringWidth(String str, const uint8_t *font)
{
  return stringWidth(str.c_str(), font);
}

// =======================================================================

int printCharX(char ch, const uint8_t *font, int x)
{
  int fwd = pgm_read_byte(font);
  int fht = pgm_read_byte(font+1);
  int offs = pgm_read_byte(font+2);
  int last = pgm_read_byte(font+3);
  if(ch<offs || ch>last) return 0;
  ch -= offs;
  int fht8 = (fht+7)/8;
  font+=4+ch*(fht8*fwd+1);
  int j,i,w = pgm_read_byte(font);
  for(j = 0; j < fht8; j++) {
    for(i = 0; i < w; i++) scr[x+LINE_WIDTH*(j+yPos)+i] = pgm_read_byte(font+1+fht8*i+j);
    if(x+i<LINE_WIDTH) scr[x+LINE_WIDTH*(j+yPos)+i]=0;
  }
  return w;
}

// =======================================================================

void printChar(unsigned char c, const uint8_t *font)
{
  if(xPos>NUM_MAX*8) return;
  int w = printCharX(c, font, xPos);
  xPos+=w+1;
}

// =======================================================================

void printString(const char *s, const uint8_t *font)
{
  while(*s) printChar(*s++, font);
  //refreshAll();
}

void printString(String str, const uint8_t *font)
{
  printString(str.c_str(), font);
}

// =======================================================================
//============================================FUNCTIONS======================================//

void drawTime0()
{
  clr();
  yPos = 0;
//  xPos = (h>9) ? 0 : 2;
  xPos = 0;
  sprintf(txt,"%02d",h);
  printString(txt, digits7x16);
  if(dots) printCharX(':', digits5x16rn, xPos);
  xPos+= 1;
  sprintf(txt,"%02d",m);
  printString(txt, digits7x16);
}

void drawTime1()
{
  clr();
  yPos = 0;
//  xPos = (h>9) ? 0 : 3;
  xPos = 0;
  sprintf(txt,"%02d",h);
  printString(txt, digits5x16rn);
  if(dots) printCharX(':', digits5x16rn, xPos);
//  xPos+=(h>=22 || h==20)?1:2;
  xPos+= 1;
  sprintf(txt,"%02d",m);
  printString(txt, digits5x16rn);
  sprintf(txt,"%02d",s);
  printString(txt, font3x7);
}

void drawTime2()
{
  clr();
  yPos = 0;
//  xPos = (h>9) ? 0 : 2;
  xPos = 0;
  sprintf(txt,"%02d",h);
  printString(txt, digits5x8rn);
  if(dots) printCharX(':', digits5x8rn, xPos);
//  xPos+=(h>=22 || h==20)?1:2;
  xPos+= 1;
  sprintf(txt,"%02d",m);
  printString(txt, digits5x8rn);
  sprintf(txt,"%02d",s);
  printString(txt, digits3x5);
  yPos = 1;
//  xPos = (day>9) ? 0 : 2;
  xPos = 0;
  sprintf(txt,"%02d&",day);
  printString(txt, font3x7);
  sprintf(txt,"%s",monthNames[month-1]);
  printString(txt, fontBesar);
  sprintf(txt,"&%d",year-2000);
  printString(txt, font3x7);
  for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;
}

void drawTime4()
{
  static uint8_t iJadwal = 0;
  static uint32_t pM;
//  uint32_t cM = millis();
  
  char jam[10];
  char TimeName[][8] = {"SUBUH","DZUHUR","ASHAR","MAGHRIB","ISYA","TANBIH"};
  int hours, minutes;  

  if (millis() - pM >= 1500) {
    pM = millis();

    // Clear the display
//    clr();

    // Skip "TERBIT" and "TRBNM" by adjusting the index
    if (iJadwal == 1) { iJadwal = 2; } // Skip "TERBIT"
    if (iJadwal == 4) { iJadwal = 5; } // Skip "TRBNM"

    // Get the current prayer time
    get_float_time_parts(times[iJadwal], hours, minutes);
//    minutes += ihti; // Adjust for local time

    // Handle minute overflow
//    if (minutes >= 60) {
//      minutes -= 60;
//      hours++;
//    }

    // Display the prayer name on the top matrix
    sprintf(txt, "%s", TimeName[iJadwal]);
    printString(txt, fontBesar); // Adjust font size as needed

    // Display the prayer time on the bottom matrix
    sprintf(jam, "%02d:%02d", hours, minutes);
    printString(jam, fontBesar); // Adjust font size as needed

    // Increment the index for the next prayer time
    iJadwal++;

    // Reset index if it exceeds the number of prayer times
    if (iJadwal > 7) {
      iJadwal = 0; // Reset to the first prayer time
    }
  }
  for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;
}


void drawTime3() {
  if (isnan(temp) || isnan(hum)) {
    // Handle sensor error
    clr();
    xPos=0;
    printString("DHT ERR.", font3x7);
    return;
  }

  // Clock part (unchanged)
  clr();
  yPos = 0;
//  xPos = (h > 9) ? 0 : 2;
  xPos = 0;
  sprintf(txt, "%02d", h);
  printString(txt, digits5x8rn);
  if (dots) printCharX(':', digits5x8rn, xPos);
//  xPos += (h >= 22 || h == 20) ? 1 : 2;
  xPos+= 1;
  sprintf(txt, "%02d", m);
  printString(txt, digits5x8rn);
  sprintf(txt, "%02d", s);
  printString(txt, digits3x5);

  // Date part with alternating temperature and humidity
  yPos = 1;
  xPos = 4;

  if (millis() - lastDisplayTime >= 5000) {
    lastDisplayTime = millis();
    showTemp = !showTemp;  // Toggle between temp and humidity
    temp = dht.readTemperature();
    hum = dht.readHumidity();
  }

  if (showTemp) sprintf(txt, "T:%dC", (int)temp);
  else          sprintf(txt, "H:%d%%", (int)hum);
  printString(txt, fontBesar);

  // Shift the display (unchanged)
  for (int i = 0; i < LINE_WIDTH; i++) {
    scr[LINE_WIDTH + i] <<= 1;
  }
}
