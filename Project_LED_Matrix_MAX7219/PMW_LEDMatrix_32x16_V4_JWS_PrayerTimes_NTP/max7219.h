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

// =========================================================================
//============================================ SETUP ======================================//

void sendCmd(int addr, byte cmd, byte data){
  digitalWrite(CS_PIN, LOW);
  for (int i = NUM_MAX-1; i>=0; i--) {
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, i==addr ? cmd : 0);
    shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, i==addr ? data : 0);
  }
  digitalWrite(CS_PIN, HIGH);
}

void sendCmdAll(byte cmd, byte data){
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

void clr(){
  for (int i = 0; i < NUM_MAX*8; i++) scr[i] = 0;
}

void scrollLeft(){
  for(int i=0; i < NUM_MAX*8+7; i++) scr[i] = scr[i+1];
}

void invert(){
  for (int i = 0; i < NUM_MAX*8; i++) scr[i] = ~scr[i];
}

void initMAX7219(){
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

int visibleCharWidth(char c, const uint8_t *font) {
  int fwd = pgm_read_byte(font);
  int fht = pgm_read_byte(font+1);
  int offs = pgm_read_byte(font+2);
  int last = pgm_read_byte(font+3);
  if (c < offs || c > last) return 0;
  c -= offs;

  int fht8 = (fht + 7) / 8;
  font += 4 + c * (fht8 * fwd + 1);

  int w = pgm_read_byte(font);
  int minX = w - 1;
  int maxX = 0;
  bool hasPixel = false;

  for (int x = 0; x < w; x++) {
    for (int y = 0; y < fht8; y++) {
      uint8_t col = pgm_read_byte(font + 1 + x * fht8 + y);
      if (col != 0) {
        hasPixel = true;
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        break;
      }
    }
  }
  return hasPixel ? (maxX - minX + 1) : 0;
}

int visualStringWidth(const char *s, const uint8_t *font) {
  int wd = 0;
  bool first = true;
  while (*s) {
    if (!first) wd += 1;  // spacing between characters
    wd += visibleCharWidth(*s++, font);
    first = false;
  }
  return wd;
}

int visualStringWidth(String str, const uint8_t *font) {
  return visualStringWidth(str.c_str(), font);
}

int printCharX(char ch, const uint8_t *font, int x){
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

void printChar(unsigned char c, const uint8_t *font){
  if(xPos>NUM_MAX*8) return;
  int w = printCharX(c, font, xPos);
  xPos+=w+1;
}

void printString(const char *s, const uint8_t *font){
  while(*s) printChar(*s++, font);
  //refreshAll();
}

void printString(String str, const uint8_t *font){
  printString(str.c_str(), font);
}

void printCentered(const char* txt, const uint8_t* font, int displayW, int displayY, int tweak = 0) {
  xPos = (displayW - visualStringWidth(txt, font)) / 2 + tweak;
  yPos = displayY;
  printString(txt, font);
}

void printCentered(String txt, const uint8_t* font, int displayW, int displayY = 0, int tweak = 0) {
  printCentered(txt.c_str(), font, displayW, displayY, tweak);
}

// =========================================================================
//============================================ FUNCTIONS ======================================//

void jamBesar(){
  static uint32_t pM;
  uint32_t cM = millis();
  static uint8_t cnt; 
   
  clr();
  yPos = 0;
  xPos = 0;
  sprintf(txt,"%02d",h);
  printString(txt, digits7x16);
  if(dots) printCharX(':', digits5x16rn, xPos);
  xPos+= 1;
  sprintf(txt,"%02d",m);
  printString(txt, digits7x16);
  
  if (cM - pM >= 1000) {
    pM = cM;
    cnt++;
    if(cnt >= 20){
      cnt = 0;
      for (int i = 0; i < LINE_WIDTH; i++) scr[LINE_WIDTH + i] <<= 1;
      tmpUtama = 1;
    }
  }
}

void jamKecilTanggal(){
  static uint32_t pM;
  uint32_t cM = millis();
  static uint8_t cnt; 
  
  if (cM - pM >= 1000) {
    pM = cM;
    cnt++;
    if(cnt >= 20){
      cnt = 0;
      for (int i = 0; i < LINE_WIDTH; i++) scr[LINE_WIDTH + i] <<= 1;
      tmpUtama = 2;
    }
  }
  
  clr();
  yPos = 0;
  xPos = 0;
  sprintf(txt,"%02d",h);
  printString(txt, digits5x8rn);
  if(dots) printCharX(':', digits5x8rn, xPos);
  xPos+= 1;
  sprintf(txt,"%02d",m);
  printString(txt, digits5x8rn);
  sprintf(txt,"%02d",s);
  printString(txt, digits3x5);
  
  if((cnt % 4) < 2){
    yPos = 1;
    xPos = 0;
    sprintf(txt,"%02d&",NTPday);
    printString(txt, font3x7);
    sprintf(txt,"%s",monthNames[NTPmonth-1]);
    printString(txt, fontBesar);
    sprintf(txt,"&%d",NTPyear-2000);
    printString(txt, font3x7);
  }
  else{    
    sprintf(txt, "%s", namaHari[NTPdayOfWeek - 1]);
    printCentered(txt, fontBesar, 32, 1, 0); 
  }
}

void jamBesarDetik(){
  static uint32_t pM;
  uint32_t cM = millis();
  static uint8_t cnt; 
  
  clr();
  yPos = 0;
  xPos = 0;
  sprintf(txt,"%02d",h);
  printString(txt, digits5x16rn);
  if(dots) printCharX(':', digits5x16rn, xPos);
  xPos+= 1;
  sprintf(txt,"%02d",m);
  printString(txt, digits5x16rn);
  sprintf(txt,"%02d",s);
  printString(txt, font3x7);
 
  if (cM - pM >= 1000) {
    pM = cM;
    cnt++;
    if(cnt >= 20){
      cnt = 0;
      for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;
      tmpUtama = 3;
    }
  }
}

void jamKecilDHT() {
  static uint32_t DHTpM; 
  static uint8_t cnt; 
  static bool showTemp;
  
  clr();
  yPos = 0;
  xPos = 0;
  sprintf(txt, "%02d", h);
  printString(txt, digits5x8rn);
  if (dots) printCharX(':', digits5x8rn, xPos);
  xPos+= 1;
  sprintf(txt, "%02d", m);
  printString(txt, digits5x8rn);
  sprintf(txt, "%02d", s);
  printString(txt, digits3x5);

  if (millis() - DHTpM >= 1500) {
    DHTpM = millis();
    showTemp = !showTemp; 
    getTempHum();
    cnt++;
    if(cnt >= 6){
      cnt = 0;
      for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;
      tmpUtama = 4;
    }
  }
  if (isnan(temp) || isnan(hum)) {
    yPos = 1;
    xPos = 1;
    printString("DHT ERR.", font3x7);
  }
  else{
//    yPos = 1;
//    xPos = 5;
    if (showTemp) sprintf(txt, "T:%dC", (int)temp);
    else          sprintf(txt, "H:%d%%", (int)hum);
  }
  printCentered(txt, fontBesar, 32, 1, 0); 
//  printString(txt, fontBesar);
}

void tampilJadwalSholat(){
  static uint8_t iJadwal = 0;
  static uint32_t pM;
  
  char jam[10];
  char namaSholat[10];
  char TimeName[][8] = {"SUBUH","TERBT","DZUHR","ASHAR","TRBNM","MAGRB","ISYA"};
  //                       0       1       2       3       4       5       6               
  int hours, minutes;  

  if (millis() - pM >= 2500) {
    pM = millis();

    clr();

    if (iJadwal == 1) { iJadwal = 2; } // Skip "TERBIT"
    if (iJadwal == 4) { iJadwal = 5; } // Skip "TRBNM"

    get_float_time_parts(times[iJadwal], hours, minutes);
    minutes += ihti;

    if (minutes >= 60) {
      minutes -= 60;
      hours++;
    }

    sprintf(namaSholat, "%s", TimeName[iJadwal]);
//    yPos = 0;
//    xPos = 3;
//    printString(namaSholat, fontBesar);
    printCentered(namaSholat, fontBesar, 32, 0, 0); 

    sprintf(jam, "%02d:%02d", hours, minutes);
//    yPos = 1;
//    xPos = 3;
//    printString(jam, digits5x8rn); 
    printCentered(jam, digits5x8rn, 32, 1, 0);


    iJadwal++;

    if (iJadwal > 7) {
      iJadwal = 0;
      tmpUtama = 0;
    }
  }
}   
