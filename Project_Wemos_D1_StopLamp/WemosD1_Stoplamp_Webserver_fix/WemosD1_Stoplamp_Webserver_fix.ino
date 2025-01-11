/*
    ___   _____    ___    ___   _        _     __  __   ___     ___   _   _   _  _   _  _   ___   _  _    ___     _____   ___  __  __  _____ 
   / __| |_   _|  / _ \  | _ \ | |      /_\   |  \/  | | _ \   | _ \ | | | | | \| | | \| | |_ _| | \| |  / __|   |_   _| | __| \ \/ / |_   _|
   \__ \   | |   | (_) | |  _/ | |__   / _ \  | |\/| | |  _/   |   / | |_| | | .` | | .` |  | |  | .` | | (_ |     | |   | _|   >  <    | |  
   |___/   |_|    \___/  |_|   |____| /_/ \_\ |_|  |_| |_|     |_|_\  \___/  |_|\_| |_|\_| |___| |_|\_|  \___|     |_|   |___| /_/\_\   |_|  

                                                                                                                                                
 "SIMPLE STOPLAMP RUNNING TEXT USING LED MATRIX WITH WEB INTERFACE" - Modified 21/10/23 by DBOSS Project
 "ADDED MODES AND SPEED CONTROL WITH OTA SOFTWARE UPDATE" - Last Modified 02/12/23 by Habib M.
  
 Based on Marco Colli MD_MAXX72XX example code
 Visit   : https://majicdesigns.github.io/MD_MAX72XX/index.html
 Support : https://www.paypal.com/paypalme/MajicDesigns/4USD

 Find DBOSS Project on YouTube
 Visit   : https://www.youtube.com/@DBOSSProject
 Support : https://saweria.co/dbossproject

 Adapted and Added OTA upload by Habib M.
 Visit   : https://www.instagram.com/hab_mz

*/

// ======== Libraries =========

#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

// ======== Pengguna Biasa : Bebas Ubah Bagian Ini =========

const int  JUMLAH_MODUL_LED_MATRIX = 4;   // Atur sesuai dengan banyaknya modul led matrix yang digunakan
const char *NAMA_WIFI     = "myStoplamp"; // Atur nama Wi-Fi stoplamp
const char *PASSWORD_WIFI = "12345678";   // Atur password yang digunakan untuk terhubung ke Wi-Fi stoplamp

// ======== Pengguna Tingkat Lanjut : Ubah Dengan Megetahui Resikonya! =========

uint8_t DEMO_DELAY = 8; 
#define DEBUG 0   

#if DEBUG
#define PRINT(s, v)   { Serial.print(F(s)); Serial.print(v); }        // Print a string followed by a value (decimal)
#define PRINTX(s, v)  { Serial.print(F(s)); Serial.print(v, HEX); }   // Print a string followed by a value (hex)
#define PRINTB(s, v)  { Serial.print(F(s)); Serial.print(v, BIN); }   // Print a string followed by a value (binary)
#define PRINTC(s, v)  { Serial.print(F(s)); Serial.print((char)v); }  // Print a string followed by a value (char)
#define PRINTS(s)     { Serial.print(F(s)); }                         // Print a string
#else
#define PRINT(s, v)   // Print a string followed by a value (decimal)
#define PRINTX(s, v)  // Print a string followed by a value (hex)
#define PRINTB(s, v)  // Print a string followed by a value (binary)
#define PRINTC(s, v)  // Print a string followed by a value (char)
#define PRINTS(s)     // Print a string
#endif

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES JUMLAH_MODUL_LED_MATRIX
#define DATA_PIN    15  // D8
#define CS_PIN      13  // D7
#define CLK_PIN     12  // D6
#define SignR       14  // D5 
#define signL       16  // D0 
#define BRAKE       5   // D1 
#define led         4   // D2 

bool blinkState;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// ========== Various Delay in miliseconds ===========

#define UNIT_DELAY      16
#define SCROLL_DELAY    (2 * UNIT_DELAY)
#define MIDLINE_DELAY   (5 * UNIT_DELAY)
#define SCANNER_DELAY   (2 * UNIT_DELAY)
#define RANDOM_DELAY    (6 * UNIT_DELAY)
#define FADE_DELAY      (3 * UNIT_DELAY)
#define SPECTRUM_DELAY  (4 * UNIT_DELAY)
#define HEARTBEAT_DELAY (1 * UNIT_DELAY)
#define HEARTS_DELAY    (28 * UNIT_DELAY)
#define EYES_DELAY      (10 * UNIT_DELAY)
#define WIPER_DELAY     (1 * UNIT_DELAY)
#define ARROWS_DELAY    (3 * UNIT_DELAY)
#define ARROWR_DELAY    (8 * UNIT_DELAY)
#define INVADER_DELAY   (6 * UNIT_DELAY)
#define PACMAN_DELAY    (4 * UNIT_DELAY)
#define SINE_DELAY      (2 * UNIT_DELAY)

#define CHAR_SPACING     1  // pixels between characters
#define BUF_SIZE        75  // character buffer size


// ========== General Variables ===========
bool sensorRead;
bool flag;
byte slct = 1;
uint32_t prevTimeAnim = 0;    // Used for remembering the millis() value in animations
uint32_t prevTimeDemo = 0;      //  Used for remembering the millis() time in demo loop
uint8_t  timeDemo = DEMO_DELAY; // number of seconds left in this demo loop
unsigned long fadeMillis=0;

char msgtext1[]="teks1";
char msgtext2[]="teks2";
const char *msgTab[]={msgtext1, msgtext2};
String inputTEXT1;
String inputTEXT2;

uint8_t kecerahan = MAX_INTENSITY/2;
uint8_t kecepatan = 2;
//uint8_t lamaAnim =  DEMO_DELAY;


// ========== Wi-Fi Configuration ===========

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

// ========== Graphic routines ===========

bool scrollText(bool bInit, const char *pmsg)
{
  static char   curMessage[BUF_SIZE];
  static char   *p = curMessage;
  static uint8_t  state = 0;
  static uint8_t  curLen, showLen;
  static uint8_t  cBuf[8];
  uint8_t         colData;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Initializing ScrollText");
    resetMatrix();
    strcpy(curMessage, pmsg);
    state = 0;
    p = curMessage;
    bInit = false;
  }

  // Is it time to scroll the text?
  if (millis()-prevTimeAnim < SCROLL_DELAY)
    return(bInit);

  // scroll the display
  mx.transform(MD_MAX72XX::TSL);  // scroll along
  prevTimeAnim = millis();        // starting point for next time

  // now run the finite state machine to control what we do
  PRINT("\nScroll FSM S:", state);
  switch (state)
  {
    case 0: // Load the next character from the font table
      PRINTC("\nLoading ", *p);
      showLen = mx.getChar(*p++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
      curLen = 0;
      state = 1;

    // !! deliberately fall through to next state to start displaying

    case 1: // display the next part of the character
      colData = cBuf[curLen++];
      mx.setColumn(0, colData);
      if (curLen == showLen)
      {
        showLen = ((*p != '\0') ? CHAR_SPACING : mx.getColumnCount()-1);
        curLen = 0;
        state = 2;
      }
      break;

    case 2: // display inter-character spacing (blank column) or scroll off the display
      mx.setColumn(0, 0);
      if (++curLen == showLen)
      {
        state = 0;
        bInit = (*p == '\0');
      }
      break;

    default:
      state = 0;
  }
  return(bInit);
}

bool graphicMidline2(bool bInit)
{
  static uint8_t  idx = 0;      // position
  static int8_t   idOffs = 1;   // increment direction

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Midline2 init");
    resetMatrix();
    idx = 0;
    idOffs = 1;
    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < MIDLINE_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  PRINT("\nML2 R:", idx);
  PRINT(" D:", idOffs);

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  // turn off the old lines
  for (uint8_t j=0; j<MAX_DEVICES; j++)
  {
    mx.setRow(j, idx, 0x00);
    mx.setRow(j, ROW_SIZE-1-idx, 0x00);
  }

  idx += idOffs;
  if ((idx == 0) || (idx == ROW_SIZE-1))
    idOffs = -idOffs;

  // turn on the new lines
  for (uint8_t j=0; j<MAX_DEVICES; j++)
  {
    mx.setRow(j, idx, 0xff);
    mx.setRow(j, ROW_SIZE-1-idx, 0xff);
  }

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return(bInit);
}

bool graphicScanner(bool bInit)
{
  const uint8_t width = 3;      // scanning bar width
  static uint8_t  idx = 0;      // position
  static int8_t   idOffs = 1;   // increment direction

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Scanner init");
    resetMatrix();
    idx = 0;
    idOffs = 1;
    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < SCANNER_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  PRINT("\nS R:", idx);
  PRINT(" D:", idOffs);

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  // turn off the old lines
  for (uint8_t i=0; i<width; i++)
    mx.setColumn(idx+i, 0);

  idx += idOffs;
  if ((idx == 0) || (idx + width == mx.getColumnCount()))
    idOffs = -idOffs;

  // turn on the new lines
  for (uint8_t i=0; i<width; i++)
   {
    mx.setColumn(idx+i, 0xff);
   }

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return(bInit);
}

bool graphicRandom(bool bInit)
{
  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Random init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < RANDOM_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time
  
  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (uint8_t i=0; i<mx.getColumnCount(); i++){
    mx.setColumn(i, (uint8_t)random(255));  
  }  
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  return(bInit);
}

bool graphicScroller(bool bInit)
{
  const uint8_t   width = 3;     // width of the scroll bar
  const uint8_t   offset = mx.getColumnCount()/3;
  static uint8_t  idx = 0;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Scroller init");
    resetMatrix();
    idx = 0;
    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < SCANNER_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  PRINT("\nS I:", idx);

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  mx.transform(MD_MAX72XX::TSL);

  mx.setColumn(0, idx>=0 && idx<width ? 0xff : 0);
  if (++idx == offset) idx = 0;

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return(bInit);
}

bool graphicSpectrum1(bool bInit)
{
  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Spectrum1 init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < SPECTRUM_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (uint8_t i=0; i<MAX_DEVICES; i++)
  {
    uint8_t r = random(ROW_SIZE);
    uint8_t cd = 0;

    for (uint8_t j=0; j<r; j++)
      cd |= 1<<j;
    for (uint8_t j=1; j<COL_SIZE-1; j++)
      mx.setColumn(i, j, ~cd);
     
  }
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return(bInit);
}

bool graphicSpectrum2(bool bInit)
{
  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Spectrum2init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < SPECTRUM_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (uint8_t i = 0; i<mx.getColumnCount(); i++)
  {
    uint8_t r = random(ROW_SIZE);
    uint8_t cd = 0;

    for (uint8_t j = 0; j<r; j++)
      cd |= 1 << j;

    mx.setColumn(i, ~cd);
  }
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return(bInit);
}

bool graphicHeartbeat(bool bInit)
{
  #define BASELINE_ROW  4

  static uint8_t  state;
  static uint8_t  r, c;
  static bool     bPoint;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Heartbeat init");
    resetMatrix();
    state = 0;
    r = BASELINE_ROW;
    c = mx.getColumnCount()-1;
    bPoint = true;
    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < HEARTBEAT_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  // now run the animation
  PRINT("\nHB S:", state);
  PRINT(" R: ", r);
  PRINT(" C: ", c);
  PRINT(" P: ", bPoint);
  mx.setPoint(r, c, bPoint);

  switch (state)
  {
    case 0: // straight line from the right side
      if (c == mx.getColumnCount()/2 + COL_SIZE)
        state = 1;
      c--;
      break;

    case 1: // first stroke
      if (r != 0) { r--; c--; }
      else state = 2;
      break;

    case 2: // down stroke
      if (r != ROW_SIZE-1) { r++; c--; }
      else state = 3;
      break;

    case 3: // second up stroke
      if (r != BASELINE_ROW) { r--; c--; }
      else state = 4;
      break;

    case 4: // straight line to the left
      if (c == 0)
      {
        c = mx.getColumnCount()-1;
        bPoint = !bPoint;
        state = 0;
      }
      else c--;
      break;

    default:
      state = 0;     
  }

  return(bInit);
}

bool graphicFade(bool bInit)
{
  static uint8_t  intensity = 0;
  static int8_t   iOffs = 1;

  // are we initializing?
  if (bInit){
    PRINTS("\n--- Fade init");
    resetMatrix();
    mx.control(MD_MAX72XX::INTENSITY, intensity);

    // Set all LEDS on
    mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
    for (uint8_t i=0; i<mx.getColumnCount(); i++){ 
      mx.setColumn(i, 0xff);
    }
    mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < FADE_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  // now run the animation
  intensity += iOffs;
  PRINT("\nF I:", intensity);
  PRINT(" D:", iOffs);
  if ((intensity == 0) || (intensity == MAX_INTENSITY))
    iOffs = -iOffs;
  mx.control(MD_MAX72XX::INTENSITY, intensity);

  return(bInit);
}

bool graphicHearts(bool bInit)
{
#define NUM_HEARTS  ((MAX_DEVICES/2) +1)
  const uint8_t heartFull[] = { 0x1c, 0x3e, 0x7e, 0xfc };
  const uint8_t heartEmpty[] = { 0x1c, 0x22, 0x42, 0x84 };
  const uint8_t offset = mx.getColumnCount()/(NUM_HEARTS+1);
  const uint8_t dataSize = (sizeof(heartFull)/sizeof(heartFull[0]));

  static bool     bEmpty;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Hearts init");
    resetMatrix();
    bEmpty = true;
    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < HEARTS_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  // now run the animation
  PRINT("\nH E:", bEmpty);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (uint8_t h=1; h<=NUM_HEARTS; h++)
  {
    for (uint8_t i=0; i<dataSize; i++)
    {
      mx.setColumn((h*offset)-dataSize+i, bEmpty ? heartEmpty[i] : heartFull[i]);
      mx.setColumn((h*offset)+dataSize-i-1, bEmpty ? heartEmpty[i] : heartFull[i]);
    }
  }
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  bEmpty = !bEmpty;

  return(bInit);
}

bool graphicEyes(bool bInit)
{
#define NUM_EYES  2
  const uint8_t eyeOpen[] = { 0x18, 0x3c, 0x66, 0x66 };
  const uint8_t eyeClose[] = { 0x18, 0x3c, 0x3c, 0x3c };
  const uint8_t offset = mx.getColumnCount()/(NUM_EYES+1);
  const uint8_t dataSize = (sizeof(eyeOpen)/sizeof(eyeOpen[0]));

  bool bOpen;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Eyes init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < EYES_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  // now run the animation
  bOpen = (random(1000) > 100);
  PRINT("\nH E:", bOpen);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (uint8_t e=1; e<=NUM_EYES; e++)
  {
    for (uint8_t i=0; i<dataSize; i++)
    {
      mx.setColumn((e*offset)-dataSize+i, bOpen ? eyeOpen[i] : eyeClose[i]);
      mx.setColumn((e*offset)+dataSize-i-1, bOpen ? eyeOpen[i] : eyeClose[i]);
    }
  }
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return(bInit);
}

bool graphicBounceBall(bool bInit)
{
  static uint8_t  idx = 0;      // position
  static int8_t   idOffs = 1;   // increment direction

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- BounceBall init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < SCANNER_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  PRINT("\nBB R:", idx);
  PRINT(" D:", idOffs);

  // now run the animation
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  // turn off the old ball
  mx.setColumn(idx, 0);
  mx.setColumn(idx+1, 0);

  idx += idOffs;
  if ((idx == 0) || (idx == mx.getColumnCount()-2))
    idOffs = -idOffs;

  // turn on the new lines
  mx.setColumn(idx, 0x18);
  mx.setColumn(idx+1, 0x18);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return(bInit);
}

bool graphicArrowScroll(bool bInit)
{
  const uint8_t arrow[] = { 0x3c, 0x66, 0xc3, 0x99 };
  const uint8_t dataSize = (sizeof(arrow)/sizeof(arrow[0]));

  static uint8_t  idx = 0;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- ArrowScroll init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < ARROWS_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  // now run the animation
  PRINT("\nAR I:", idx);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.transform(MD_MAX72XX::TSL);
  mx.setColumn(0, arrow[idx++]);
  if (idx == dataSize) idx = 0;
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return(bInit);
}

bool graphicWiper(bool bInit)
{
  static uint8_t  idx = 0;      // position
  static int8_t   idOffs = 1;   // increment direction

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Wiper init");
    resetMatrix();
    bInit = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < WIPER_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  PRINT("\nW R:", idx);
  PRINT(" D:", idOffs);

  // now run the animation
  mx.setColumn(idx, idOffs == 1 ? 0xff : 0);
  idx += idOffs;
  if ((idx == 0) || (idx == mx.getColumnCount()))
    idOffs = -idOffs;

  return(bInit);
}

bool graphicInvader(bool bInit)
{
  const uint8_t invader1[] = { 0x0e, 0x98, 0x7d, 0x36, 0x3c };
  const uint8_t invader2[] = { 0x70, 0x18, 0x7d, 0xb6, 0x3c };
  const uint8_t dataSize = (sizeof(invader1)/sizeof(invader1[0]));

  static int8_t idx;
  static bool   iType;

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Invader init");
    resetMatrix();
    bInit = false;
    idx = -dataSize;
    iType = false;
  }

  // Is it time to animate?
  if (millis()-prevTimeAnim < INVADER_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  // now run the animation
  PRINT("\nINV I:", idx);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear();
  for (uint8_t i=0; i<dataSize; i++)
  {
    mx.setColumn(idx-dataSize+i, iType ? invader1[i] : invader2[i]);
    mx.setColumn(idx+dataSize-i-1, iType ? invader1[i] : invader2[i]);
  }
  idx++;
  if (idx == mx.getColumnCount()+(dataSize*2)) bInit = true;
  iType = !iType;
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return(bInit);
}

bool graphicPacman(bool bInit)
{
  #define MAX_FRAMES  4   // number of animation frames
  #define PM_DATA_WIDTH  18
  const uint8_t pacman[MAX_FRAMES][PM_DATA_WIDTH] =  // ghost pursued by a pacman
  {
    { 0x3c, 0x7e, 0x7e, 0xff, 0xe7, 0xc3, 0x81, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe },
    { 0x3c, 0x7e, 0xff, 0xff, 0xe7, 0xe7, 0x42, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe },
    { 0x3c, 0x7e, 0xff, 0xff, 0xff, 0xe7, 0x66, 0x24, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe },
    { 0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe },
  };

  static int16_t idx;        // display index (column)
  static uint8_t frame;      // current animation frame
  static uint8_t deltaFrame; // the animation frame offset for the next frame

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Pacman init");
    resetMatrix();
    bInit = false;
    idx = -1; //DATA_WIDTH;
    frame = 0;
    deltaFrame = 1;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < PACMAN_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  PRINT("\nPAC I:", idx);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear();

  // clear old graphic
  for (uint8_t i = 0; i<PM_DATA_WIDTH; i++)
    mx.setColumn(idx - PM_DATA_WIDTH + i, 0);
  // move reference column and draw new graphic
  idx++;
  for (uint8_t i = 0; i<PM_DATA_WIDTH; i++)
    mx.setColumn(idx - PM_DATA_WIDTH + i, pacman[frame][i]);

  // advance the animation frame
  frame += deltaFrame;
  if (frame == 0 || frame == MAX_FRAMES - 1)
    deltaFrame = -deltaFrame;

  // check if we are completed and set initialize for next time around
  if (idx == mx.getColumnCount() + PM_DATA_WIDTH) bInit = true;

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return(bInit);
}

bool graphicArrowRotate(bool bInit)
{
  static uint16_t idx;        // transformation index

  uint8_t arrow[COL_SIZE] =
  {
    0b00000000,
    0b00011000,
    0b00111100,
    0b01111110,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00000000
  };

  MD_MAX72XX::transformType_t  t[] =
  {
    MD_MAX72XX::TRC, MD_MAX72XX::TRC,
    MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR,
    MD_MAX72XX::TRC, MD_MAX72XX::TRC,
    MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL,
    MD_MAX72XX::TRC,
  };

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- ArrowRotate init");
    resetMatrix();
    bInit = false;
    idx = 0;

    // use the arrow bitmap
    mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
    for (uint8_t j = 0; j<mx.getDeviceCount(); j++)
      mx.setBuffer(((j + 1)*COL_SIZE) - 1, COL_SIZE, arrow);
    mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < ARROWR_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::ON);
  mx.transform(t[idx++]);
  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::OFF);

  // check if we are completed and set initialize for next time around
  if (idx == (sizeof(t) / sizeof(t[0]))) bInit = true;

  return(bInit);
}

bool graphicSinewave(bool bInit)
{
  static uint8_t curWave = 0;
  static uint8_t idx;

  #define SW_DATA_WIDTH  11    // valid data count followed by up to 10 data points
  const uint8_t waves[][SW_DATA_WIDTH] =
  {
    {  9,   8,  6,   1,   6,  24,  96, 128,  96,  16,   0 },
    {  6,  12,  2,  12,  48,  64,  48,   0,   0,   0,   0 },
    { 10,  12,   2,   1,   2,  12,  48,  64, 128,  64, 48 },

  };
  const uint8_t WAVE_COUNT = sizeof(waves) / (SW_DATA_WIDTH * sizeof(uint8_t));

  // are we initializing?
  if (bInit)
  {
    PRINTS("\n--- Sinewave init");
    resetMatrix();
    bInit = false;
    idx = 1;
  }

  // Is it time to animate?
  if (millis() - prevTimeAnim < SINE_DELAY)
    return(bInit);
  prevTimeAnim = millis();    // starting point for next time

  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::ON);
  mx.transform(MD_MAX72XX::TSL);
  mx.setColumn(0, waves[curWave][idx++]);
  if (idx > waves[curWave][0])
  {
    curWave = random(WAVE_COUNT);
    idx = 1;
  }
  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::OFF);

  return(bInit);
}

// ========== Control routines ===========


void resetMatrix(void)
{
  mx.control(MD_MAX72XX::INTENSITY, kecerahan);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  mx.clear();
  prevTimeAnim = 0;
}

void runMatrixAnimation(void){
  static  uint8_t state = 0;
  static  uint8_t mesg = 0;
  static  boolean bRestart = true;
  static boolean  bInMessages = false;
  boolean changeState = false;
    
  if (millis()-prevTimeDemo >= 1000){
    prevTimeDemo = millis();
    blinkState = !blinkState;
    if (--timeDemo <= 0 && !bRestart){
      timeDemo = DEMO_DELAY;
      changeState = true;
    }
  }
  
  if (changeState)
  {
    if (bInMessages) // the message display state
    {
      mesg++;
      if (mesg >= sizeof(msgTab)/sizeof(msgTab[0]))
      {
        mesg = 0;
        bInMessages = false;
        state++;
      }
    }else{
      state++;
      bRestart = true;
    }
  }

  if (slct == 1){
    switch(state)  { 
      case 0 : bInMessages = true; 
               bRestart = scrollText(bRestart, msgTab[mesg]); 
               break;
      default : state = 0;             
    }
  }
  if (slct == 2){
    switch(state){
      case  1: bRestart = graphicMidline2(bRestart);       break;
      case  2: bRestart = graphicScanner(bRestart);        break;
      case  3: bRestart = graphicRandom(bRestart);         break;
      case  4: bRestart = graphicFade(bRestart);           break;
      case  5: bRestart = graphicSpectrum1(bRestart);      break;
      case  6: bRestart = graphicHeartbeat(bRestart);      break;
      case  7: bRestart = graphicHearts(bRestart);         break;
      case  8: bRestart = graphicEyes(bRestart);           break;
      case  9: bRestart = graphicBounceBall(bRestart);     break;
      case 10: bRestart = graphicArrowScroll(bRestart);    break;
      case 11: bRestart = graphicScroller(bRestart);       break;
      case 12: bRestart = graphicWiper(bRestart);          break;
      case 13: bRestart = graphicInvader(bRestart);        break;
      case 14: bRestart = graphicPacman(bRestart);         break;
      case 15: bRestart = graphicArrowRotate(bRestart);    break;
      case 16: bRestart = graphicSpectrum2(bRestart);      break;
      case 17: bRestart = graphicSinewave(bRestart);       break;
      default: state = 1; 
    }
  }
  if (slct == 3){
    switch(state){
      case  0: bInMessages = true; 
               bRestart = scrollText(bRestart, msgTab[mesg]);  break;
      case  1: bRestart = graphicMidline2(bRestart);       break;
      case  2: bRestart = graphicScanner(bRestart);        break;
      case  3: bRestart = graphicRandom(bRestart);         break;
      case  4: bRestart = graphicFade(bRestart);           break;
      case  5: bRestart = graphicSpectrum1(bRestart);      break;
      case  6: bRestart = graphicHeartbeat(bRestart);      break;
      case  7: bRestart = graphicHearts(bRestart);         break;
      case  8: bRestart = graphicEyes(bRestart);           break;
      case  9: bRestart = graphicBounceBall(bRestart);     break;
      case 10: bRestart = graphicArrowScroll(bRestart);    break;
      case 11: bRestart = graphicScroller(bRestart);       break;
      case 12: bRestart = graphicWiper(bRestart);          break;
      case 13: bRestart = graphicInvader(bRestart);        break;
      case 14: bRestart = graphicPacman(bRestart);         break;
      case 15: bRestart = graphicArrowRotate(bRestart);    break;
      case 16: bRestart = graphicSpectrum2(bRestart);      break;
      case 17: bRestart = graphicSinewave(bRestart);       break;
      default: state = 0;
    }
  }
}


// ======== WEB Interface =========

void handleRoot() {
  String T1=inputTEXT1;
  String T2=inputTEXT2;
  uint8_t cepat=kecepatan;
  uint8_t bright=kecerahan;
  uint8_t waktuAnim=DEMO_DELAY;

  String html="<!DOCTYPE html>";
  html=html+"<html lang='en'>";
  html=html+"<head>";
  html=html+"<meta charset='UTF-8'>";
  html=html+"<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html=html+"<title>Settings Page</title>";
  html=html+"<style>";
  html=html+"body {";
  html=html+"    font-family: Arial, sans-serif;";
  html=html+"    background-color: lightslategray";
  html=html+"    display: flex;";
  html=html+"    justify-content: center;";
  html=html+"    align-items: center;";
  html=html+"    height: 150vh;";
  html=html+"    margin: 0;";
  html=html+"}";
  html=html+".container {";
  html=html+"    background-color: gray;";
  html=html+"    padding: 20px;";
  html=html+"    border-radius: 10px;";
  html=html+"    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);";
  html=html+"    max-width: 600px;";
  html=html+"    width: 100%;";
  html=html+"}";
  html=html+"h1, h2 {";
  html=html+"    color: #333;";
  html=html+"    text-align: center;";
  html=html+"}";
  html=html+"table {";
  html=html+"    width: 100%;";
  html=html+"    border-collapse: collapse;";
  html=html+"    margin-top: 20px;";
  html=html+"}";
  html=html+"table, th, td {";
  html=html+"    border: 1px solid #ccc;";
  html=html+"}";
  html=html+"th, td {";
  html=html+"    padding: 10px;";
  html=html+"    text-align: center;";
  html=html+"}";
  html=html+"input[type='text'] {";
  html=html+"    width: 100%;";
  html=html+"    padding: 8px;";
  html=html+"    margin-bottom: 10px;";
  html=html+"    border: 1px solid #ccc;";
  html=html+"    border-radius: 4px;";
  html=html+"    box-sizing: border-box;";
  html=html+"}";
  html=html+"input[type='radio'] {";
  html=html+"    margin-right: 5px;";
  html=html+"}";
  html=html+"input[type='submit'] {";
  html=html+"    background-color: #4CAF50;";
  html=html+"    color: #fff;";
  html=html+"    padding: 10px 20px;";
  html=html+"    border: none;";
  html=html+"    border-radius: 4px;";
  html=html+"    cursor: pointer;";
  html=html+"    font-size: 16px;";
  html=html+"    width: 100%;";
  html=html+"}";
  html=html+"input[type='submit']:hover {";
  html=html+"    background-color: #45a049;";
  html=html+"}";
  html=html+"h5 a {";
  html=html+"    color: #007BFF;";
  html=html+"    text-decoration: none;";
  html=html+"    display: block;";
  html=html+"    text-align: center;";
  html=html+"}";
  html=html+"</style>";
  html=html+"</head>";
  html=html+"<body>";
  html=html+"<div class='container'>";    
  html=html+"    <h1>SETTING MENU</h1>"; 
  html=html+"    <h2>STOPLAMP RUNNING TEXT VARIO</h3>"; 
  html=html+"    <form action='/overwriteSettings' method='get'>"; 
  html=html+"        <table>"; 
  html=html+"            <tr>"; 
  html=html+"                <td colspan='2'>"; 
  html=html+"                    <label>Pilih Mode :</label>"; 
  html=html+"                    <br><br>"; 
  html=html+"                    <label><input type='radio' id='slc1' name='selection' value='1' > Running Teks </label>"; 
  html=html+"                    <label><input type='radio' id='slc2' name='selection' value='2'> Animasi </label>"; 
  html=html+"                    <label><input type='radio' id='slc3' name='selection' value='3'checked> Kombinasi </label>"; 
  html=html+"                    <label><input type='radio' id='slc4' name='selection' value='4'> Manual </label>"; 
  html=html+"                </td>"; 
  html=html+"            <tr>"; 
  html=html+"                <td style='width:25%' ><b>Teks 1</b></td>"; 
  html=html+"                <td style='width:75%' ><input type='text' name='text1' value='"+T1+"' maxlength='80'></td>"; 
  html=html+"            </tr>"; 
  html=html+"            <tr>"; 
  html=html+"                <td style='width:25%' ><b>Teks 2</b></td>"; 
  html=html+"                <td style='width:75%' ><input type='text' name='text2' value='"+T2+"' maxlength='80'></td>"; 
  html=html+"            </tr>"; 
  html=html+"            </tr>"; 
  html=html+"            <td colspan='2'>"; 
  html=html+"               <label>Kecepatan Teks (0-30ms) : "+cepat+"</label>"; 
  html=html+"                <br>"; 
  html=html+"                <label><input type='range' id='RangeCepat' name='kecepatan' value='"+cepat+"' min='0' max='30'> </label>"; 
  html=html+"                <br><br>"; 
  html=html+"                <label>Kecerahan (0-15) : "+bright+"</label>"; 
  html=html+"                <br>"; 
  html=html+"                <label><input type='range' id='RangeCerah' name='Kecerahan' value='"+bright+"' min='0' max='15'> </label>"; 
  html=html+"                <br><br>"; 
  html=html+"                <label>Lama Tiap Animasi (0-20s) : "+waktuAnim+"</label>"; 
  html=html+"                <br>"; 
  html=html+"                <label><input type='range' id='rangeEfek' name='Efek' value='"+waktuAnim+"' min='0' max='20'> </label>"; 
  html=html+"            </td>"; 
  html=html+"        </table>";
  html=html+"        <table>";
  html=html+"            <tr><td colspan='2'>"; 
  html=html+"                <label>Animasi (Pilih Mode Manual): </label>"; 
  html=html+"                <br><br>"; 
  html=html+"                <label><input type='checkbox' id='man1' name='man1' value='1'> 1</label>"; 
  html=html+"                <label><input type='checkbox' id='man2' name='man2' value='1'> 2</label>"; 
  html=html+"                <label><input type='checkbox' id='man3' name='man3' value='1'> 3</label>"; 
  html=html+"                <label><input type='checkbox' id='man4' name='man4' value='1'> 4</label>"; 
  html=html+"                <label><input type='checkbox' id='man5' name='man5' value='1'> 5</label>"; 
  html=html+"                <label><input type='checkbox' id='man6' name='man6' value='1'> 6</label>"; 
  html=html+"                <label><input type='checkbox' id='man7' name='man7' value='1'> 7</label>"; 
  html=html+"                <label><input type='checkbox' id='man8' name='man8' value='1'> 8</label>"; 
  html=html+"                <label><input type='checkbox' id='man9' name='man9' value='1'> 9</label>"; 
  html=html+"                <label><input type='checkbox' id='man10' name='man10' value='1'> 10</label>"; 
  html=html+"                <label><input type='checkbox' id='man11' name='man11' value='1'> 11</label>"; 
  html=html+"                <label><input type='checkbox' id='man12' name='man12' value='1'> 12</label>"; 
  html=html+"            </td>"; 
  html=html+"            </tr>"; 
  html=html+"            <td colspan='2'><label>Kecepatan Tiap Animasi :</label></td>"; 
  html=html+"            <tr>"; 
  html=html+"                <td >"; 
  html=html+"                    <label>Animasi 1:<input type='range' id='rangeEfek' name='Efek1' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 2:<input type='range' id='rangeEfek' name='Efek2' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 3:<input type='range' id='rangeEfek' name='Efek3' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 4:<input type='range' id='rangeEfek' name='Efek4' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 5:<input type='range' id='rangeEfek' name='Efek5' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 6:<input type='range' id='rangeEfek' name='Efek6' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 7:<input type='range' id='rangeEfek' name='Efek7' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 8:<input type='range' id='rangeEfek' name='Efek8' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 9:<input type='range' id='rangeEfek' name='Efek9' min='0' max='30'> </label>"; 
  html=html+"                   <label>Animasi 10:<input type='range' id='rangeEfek' name='Efek10' min='0' max='30'> </label>"; 
  html=html+"                </td>"; 
  html=html+"                <td>"; 
  html=html+"                    <label>Animasi 11:<input type='range' id='rangeEfek' name='Efek11' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 12:<input type='range' id='rangeEfek' name='Efek12' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 13:<input type='range' id='rangeEfek' name='Efek13' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 14:<input type='range' id='rangeEfek' name='Efek14' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 15:<input type='range' id='rangeEfek' name='Efek15' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 16:<input type='range' id='rangeEfek' name='Efek16' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 17:<input type='range' id='rangeEfek' name='Efek17' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 18:<input type='range' id='rangeEfek' name='Efek18' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 19:<input type='range' id='rangeEfek' name='Efek19' min='0' max='30'> </label>"; 
  html=html+"                    <label>Animasi 20:<input type='range' id='rangeEfek' name='Efek20' min='0' max='30'> </label>"; 
  html=html+"                </td>"; 
  html=html+"            </tr>"; 
  html=html+"        </table>";
  html=html+"        <table>";
  html=html+"            <tr>";
  html=html+"                <td colspan='2'>";
  html=html+"                    <input type='submit' name='Save' value='Submit'>";
  html=html+"                </td>";
  html=html+"            </tr>";
  html=html+"            <tr>";
  html=html+"                <td colspan='2'>";
  html=html+"                   <h5><a href='http://192.168.4.1/update'>© 2023 vario Stoplamp</a></h5>";
  html=html+"                   <h5>";
  html=html+"                     <a href='https://www.youtube.com/@DBOSSProject'>@DBOSSProject</a> x";
  html=html+"                     <a href='https://www.instagram.com/Hab_mz'>@Hab_mz</a>";
  html=html+"                   </h5>";
  html=html+"                </td>";
  html=html+"            </tr>";
  html=html+"        </table>";
  html=html+"    </form>";
  html=html+"</div>";
  html=html+"</body>";
  html=html+"</html>";
  server.send(200,"text/html",html);
}
// ======== EEPROM =========

void overwriteSettings(){
  String text1 = server.arg("text1");
  write_EEPROM(101, 200, text1);
  
  String text2 = server.arg("text2");
  write_EEPROM(201, 300, text2);
  
  String selection = server.arg("selection");
  write_EEPROM(301, 302, selection);
  
  String spd = server.arg("kecepatan");
  write_EEPROM(303, 305, spd);
  
  String brig = server.arg("Kecerahan");
  write_EEPROM(306, 308, brig);
  
  String anim = server.arg("Efek");
  write_EEPROM(309, 311, anim);

  static String mod;
  if(selection == "1"){
    mod = "Text only";
  }else if(selection == "2"){
    mod = "Animation only";
  }else if(selection == "3"){
    mod = "Text and Animation";
  }else{
    mod = "Manual Mode";
  }
  String html="<!DOCTYPE html>";
  html=html+"<html lang='en'>";
  html=html+"<head>";
  html=html+"<meta charset='UTF-8'>";
  html=html+"<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html=html+"<title>Done page</title>";
  html=html+"<style>";
  html=html+"body {";
  html=html+"    font-family: Arial, sans-serif;";
  html=html+"    text-align: center;";
  html=html+"    background-color: #f4f4f4;";
  html=html+"    display: flex;";
  html=html+"    justify-content: center;";
  html=html+"    align-items: center;";
  html=html+"    height: 100vh;";
  html=html+"    margin: 0;";
  html=html+"}";
  html=html+".container {";
  html=html+"    background-color: #fff;";
  html=html+"    padding: 20px;";
  html=html+"    border-radius: 10px;";
  html=html+"    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);";
  html=html+"    max-width: 600px;";
  html=html+"    width: 100%;";
  html=html+"}";
  html=html+"h1, h2 {";
  html=html+"    color: #333;";
  html=html+"}";
  html=html+"</style>";
  html=html+"</head>";
  html=html+"<body>";
  html=html+"<div class='container'>";
  html=html+"    <h1>Done!</h1>";
  html=html+"    <h2>Teks 1 : "+text1+"</h2>";
  html=html+"    <h2>Teks 2 : "+text2+"</h2>";
  html=html+"    <h2>Mode Tampilan : "+mod+"</h2>";
  html=html+"</div>";
  html=html+"</body>";
  html=html+"</html>";
  server.send(200, "text/html", html);  reload_EEPROM();
//  bInMessages = true;
}

void write_EEPROM(int pos1, int pos2,String strText){
  int pj_strText=strText.length()+1;
  char stringIn[pj_strText];
  strText.toCharArray(stringIn,pj_strText);

  int j=-1;
  for(int i=pos1; i<pos2+1; i++){
    j++;
    if (i<pos1+pj_strText )  {
      EEPROM.write(i, stringIn[j]);
      EEPROM.commit();
    }else{
      EEPROM.write(i, '\0');
      EEPROM.commit();
    }
  }
}

String read_EEPROM(int pos1,int pos2){ 
  int i;
  char c;
  String temp="";
  for (i=pos1; i<pos2; i++){
    c=EEPROM.read(i);
    temp=temp+String(c);
  }
  temp=temp+'\0';
  return temp;
}

void reload_EEPROM(){
  inputTEXT1  = read_EEPROM(101,200);
  inputTEXT1  = inputTEXT1.substring(0, inputTEXT1.length());

  inputTEXT2  = read_EEPROM(201,300);
  inputTEXT2  = inputTEXT2.substring(0,inputTEXT2.length());

  String temp = read_EEPROM(301,302);
  temp = temp.substring(0,temp.length());
  slct = temp.toInt();

  
  String spd2 = read_EEPROM(303, 305);
  temp = spd2.substring(0,temp.length());
  kecepatan = spd2.toInt();
  
  String brig2 = read_EEPROM(306, 308);
  temp = brig2.substring(0,temp.length());
  kecerahan = brig2.toInt();
  
  String anim2 = read_EEPROM(309, 311);
  temp = anim2.substring(0,temp.length());
  DEMO_DELAY = anim2.toInt();
}

void stopLight(){
  if (!flag){
    mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
    for (uint8_t i=0; i<mx.getColumnCount(); i++){ 
      mx.setColumn(i, 0xff);
    }
    mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
    digitalWrite(led, HIGH);
    flag = true;
  } 
}

void startBlink(){
  if(blinkState){ digitalWrite(led, HIGH); }
  else { digitalWrite(led, LOW); }
  } 

// ======== Main Program =========

void setup(){
  #if DEBUG
    Serial.begin(115200);
  #endif 
  EEPROM.begin(512);

  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  pinMode(BRAKE, INPUT_PULLUP);
  
  mx.begin();  
  prevTimeAnim = millis();
  prevTimeDemo = millis();
  
  WiFi.softAP(NAMA_WIFI, PASSWORD_WIFI);
  IPAddress myIP = WiFi.softAPIP();
  server.on("/", handleRoot);
  server.on("/overwriteSettings",overwriteSettings);
  httpUpdater.setup(&server);
  server.begin();
    
  bool hasRunFlag = EEPROM.read(0);
  
  if (hasRunFlag != 1) {
    String text1 = "lorem ipsum dolor sit amet";
    write_EEPROM(101, 200, text1);
    String text2 = "stoplamp running nih bos";
    write_EEPROM(201, 300, text2);
    String selection = "3";
    write_EEPROM(301, 302, selection);
   
    EEPROM.write(0, 1);
    EEPROM.commit();
  }
  reload_EEPROM();  
}

void loop(){
  String str0 = inputTEXT1; 
  int str_len0 = str0.length() + 1; 
  char msgtext01[str_len0];
  str0.toCharArray(msgtext01, str_len0);
  
  String str1 = inputTEXT2; 
  int str_len1 = str1.length() + 1; 
  char msgtext02[str_len1];
  str1.toCharArray(msgtext02, str_len1);
  
  msgTab[0]=msgtext01;
  msgTab[1]=msgtext02;
 
  server.handleClient();
 
  static bool brakeStat = false;
  sensorRead = digitalRead(BRAKE);

  if(!sensorRead){
   brakeStat = true;
   stopLight();
  }else{
    flag = false;  
    digitalWrite(led, HIGH);
    if(brakeStat){
      brakeStat = false;
      mx.clear();
    }
    runMatrixAnimation();
    startBlink();
  }
}
