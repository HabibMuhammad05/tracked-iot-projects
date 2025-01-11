#include <Arduino.h>
#include <U8g2lib.h>

#include<esp_now.h>
#include<WiFi.h>

uint8_t broadcastAddress[]={0xCC,0x50,0xE3,0xF0,0x1E,0x36};

typedef struct Pesan{
  bool stat[14];
  int joyData[4];
//  bool rs[4];
}Pesan;

Pesan pesan;

esp_now_peer_info_t slave;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status){
  Serial.print("Status terakhir:  ");
  Serial.println(status==ESP_NOW_SEND_SUCCESS?"Terkirim":"Gagal");
}

U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 22, /* data=*/ 23, /* cs=*/ U8X8_PIN_NONE, /* dc=*/ 21, /* reset=*/ 19);

static const unsigned char image_GameMode_bits[] U8X8_PROGMEM = {0x20,0x00,0xfe,0x03,0xfb,0x07,0x71,0x05,0xfb,0x07,0x8f,0x07,0x07,0x07,0x03,0x06};
static const unsigned char image_Bluetooth_Connected_bits[] U8X8_PROGMEM = {0x04,0x00,0x0d,0x00,0x16,0x60,0x4c,0x97,0x4c,0x97,0x16,0x60,0x0d,0x00,0x04,0x00};
static const unsigned char image_arrow_up_bits[] U8X8_PROGMEM = {0x04,0x0e,0x15,0x04,0x04,0x04,0x04};
static const unsigned char image_arrow_down_bits[] U8X8_PROGMEM = {0x04,0x04,0x04,0x04,0x15,0x0e,0x04};
static const unsigned char image_arrow_right_bits[] U8X8_PROGMEM = {0x10,0x20,0x7f,0x20,0x10};
static const unsigned char image_arrow_left_bits[] U8X8_PROGMEM = {0x04,0x02,0x7f,0x02,0x04};
static const unsigned char image_InfraredArrowUp_bits[] U8X8_PROGMEM = {0x18,0x3c,0x7e,0xff};
static const unsigned char image_InfraredArrowDown_bits[] U8X8_PROGMEM = {0xff,0x7e,0x3c,0x18};
static const unsigned char image_ButtonRight_bits[] U8X8_PROGMEM = {0x01,0x03,0x07,0x0f,0x07,0x03,0x01};
static const unsigned char image_ButtonLeft_bits[] U8X8_PROGMEM = {0x08,0x0c,0x0e,0x0f,0x0e,0x0c,0x08};

uint8_t button[14] = {32, 33, 25, 26, 27, 14, 12, 13, 18, 15, 5, 4, 17, 16  }; // "L3", "R3", "L2", "L1", "UP", "LEFT", "DOWN", "RIGHT", "SQUARE", "CROSS", "ROUND", "TRIANGLE", "R2", "R1"
unsigned long previousMillis = 0;        // will store last time LED was updated

// Define joystick pins
const int joy1XPin = 36; // Joystick 1 X-axis
const int joy1YPin = 39; // Joystick 1 Y-axis
const int joy2XPin = 34; // Joystick 2 X-axis
const int joy2YPin = 35; // Joystick 2 Y-axis

// Define box dimensions, +16 Pixel to print
const int boxSize = 32;
const int joy1XBox = 48;
const int joy1YBox = 48;
const int joy2XBox = 80;
const int joy2YBox = 48;

// Calibration variables
int joy1XCenter = 0;
int joy1YCenter = 0;
int joy2XCenter = 0;
int joy2YCenter = 0;

// Threshold value to create a dead zone around the center
const int centerThreshold = 10;

void setup() {

  u8g2.begin();
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  for(uint8_t i = 0; i < 14; i++){
    pinMode(button[i], INPUT);
  }
  
  WiFi.mode(WIFI_STA);
  if(esp_now_init()!=ESP_OK){
    Serial.println("ESPNow Gagal!");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
  memcpy(slave.peer_addr, broadcastAddress, 6);
  slave.channel = 0;  
  slave.encrypt = false;       
  if(esp_now_add_peer(&slave) != ESP_OK){
    Serial.println("Gagal menambahkan slave!");
    return;
  }
  
  // Calibration phase
  calibrateJoysticks();
  
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);
  u8g2.setFont(u8g2_font_t0_18b_tr);
  u8g2.drawStr(0, 13, "LEXARGA");
  u8g2.drawXBMP(68, 3, 11, 8, image_GameMode_bits);
  u8g2.drawXBMP(81, 3, 16, 8, image_Bluetooth_Connected_bits);
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(100, 7, "v:24v");
  u8g2.drawStr(100, 13, "v:3.3v");
  u8g2.drawLine(0, 15, 128, 15);
  u8g2.sendBuffer();
}

void loop() {
  unsigned long startTime = millis();

//  if (millis() - previousMillis >= 1000) {
//    previousMillis = millis();
//    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
//  }
  
  int joy1XValue = analogRead(joy1XPin) - joy1XCenter;
  int joy1YValue = analogRead(joy1YPin) - joy1YCenter;
  int joy2XValue = analogRead(joy2XPin) - joy2XCenter;
  int joy2YValue = analogRead(joy2YPin) - joy2YCenter;
  
  pesan.joyData[0] = joy1XValue;
  pesan.joyData[1] = joy1YValue;
  pesan.joyData[2] = joy2XValue;
  pesan.joyData[3] = joy2YValue;

  Serial.print(String(joy1XValue) + ", " + String(joy1YValue) + ", " + String(joy2XValue) + ", " + String(joy2YValue) + "-----");
  Serial.print("-----");
  
  // Apply threshold to create a dead zone
  joy1XValue = abs(joy1XValue) > centerThreshold ? joy1XValue : 0;
  joy1YValue = abs(joy1YValue) > centerThreshold ? joy1YValue : 0;
  joy2XValue = abs(joy2XValue) > centerThreshold ? joy2XValue : 0;
  joy2YValue = abs(joy2YValue) > centerThreshold ? joy2YValue : 0;

  // Map joystick values to box positions
  int joy1XPos = map(joy1XValue, -2048, 2048, joy1XBox - boxSize / 2, joy1XBox + boxSize / 2);
  int joy1YPos = map(joy1YValue, -2048, 2048, joy1YBox - boxSize / 2, joy1YBox + boxSize / 2);
  int joy2XPos = map(joy2XValue, -2048, 2048, joy2XBox - boxSize / 2, joy2XBox + boxSize / 2);
  int joy2YPos = map(joy2YValue, -2048, 2048, joy2YBox - boxSize / 2, joy2YBox + boxSize / 2);
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB08_tr);
  
   for(uint8_t i = 0; i < 14; i++){
     pesan.stat[i] = digitalRead(button[i]);
     if(pesan.stat[i]){
       switch(i){
         case 0  : u8g2.drawStr(11, 28, "------L3         --------");        break;
         case 1  : u8g2.drawStr(11, 28, "--------         ------R3");        break;
         case 2  : u8g2.drawStr(11, 28, "---L2---         --------");        break;
         case 3  : u8g2.drawStr(11, 28, "L1------         --------");        break;
         case 4  : u8g2.drawXBMP(19, 45, 5, 7, image_arrow_up_bits);         break;
         case 5  : u8g2.drawXBMP(13, 51, 7, 5, image_arrow_left_bits);       break;
         case 6  : u8g2.drawXBMP(19, 55, 5, 7, image_arrow_down_bits);       break;
         case 7  : u8g2.drawXBMP(23, 51, 7, 5, image_arrow_right_bits);      break;
         case 8  : u8g2.drawXBMP(98, 50, 4, 7, image_ButtonLeft_bits);       break;
         case 9  : u8g2.drawXBMP(103, 58, 8, 4, image_InfraredArrowDown_bits); break;
         case 10 : u8g2.drawXBMP(112, 50, 4, 7, image_ButtonRight_bits);     break;
         case 11 : u8g2.drawXBMP(103, 45, 8, 4, image_InfraredArrowUp_bits); break;
         case 12 : u8g2.drawStr(11, 28, "--------         ---R2---");        break;
         case 13 : u8g2.drawStr(11, 28, "--------         R1------");        break;
         default : u8g2.drawStr(11, 28, "--------         --------");        break;
       }
     }
     Serial.print(digitalRead(button[i]));
     Serial.print(", ");
  }
  u8g2.drawStr(11, 28, "--------         --------");
  u8g2.drawFrame(11, 43, 21, 21);
  u8g2.drawFrame(96, 42, 22, 22);
  u8g2.drawFrame(joy1XBox - boxSize / 2, joy1YBox - boxSize / 2, boxSize, boxSize);
  u8g2.drawFrame(joy2XBox - boxSize / 2, joy2YBox - boxSize / 2, boxSize, boxSize);
  u8g2.drawDisc(joy1XPos-1, joy1YPos, 3, U8G2_DRAW_ALL);
  u8g2.drawDisc(joy2XPos-1, joy2YPos, 3, U8G2_DRAW_ALL);
  
//  u8g2.sendBuffer();
  u8g2.updateDisplayArea(0, 2, 16, 6);  //tile_area_x_pos, tile_area_y_pos, tile_area_width, tile_area_height
  
  cekButton();
  unsigned long endTime = millis();
  Serial.print("------Update time (miliSeconds): ");
  Serial.print(endTime - startTime);
  Serial.print("-----");
  delay(50);
}

void calibrateJoysticks() {
  Serial.println("Calibrating joysticks...");

  // Take multiple readings to determine the center position
  int numSamples = 50;
  long joy1XSum = 0, joy1YSum = 0;
  long joy2XSum = 0, joy2YSum = 0;

  for (int i = 0; i < numSamples; i++) {
    joy1XSum += analogRead(joy1XPin);
    joy1YSum += analogRead(joy1YPin);
    joy2XSum += analogRead(joy2XPin);
    joy2YSum += analogRead(joy2YPin);
    delay(10);
  }

  // Calculate the average center position
  joy1XCenter = joy1XSum / numSamples;
  joy1YCenter = joy1YSum / numSamples;
  joy2XCenter = joy2XSum / numSamples;
  joy2YCenter = joy2YSum / numSamples;

  Serial.println("Calibration complete.");
  Serial.print("Joystick 1 Center: ");
  Serial.print(joy1XCenter);
  Serial.print(", ");
  Serial.println(joy1YCenter);
  Serial.print("Joystick 2 Center: ");
  Serial.print(joy2XCenter);
  Serial.print(", ");
  Serial.println(joy2YCenter);
}

void cekButton(){
//  for(int i=0; i<14; i++){
//    if((!digitalRead(button[i]))){
//      while(!digitalRead(button[i]));
//      delay(100);
//      r[i]=!r[i]; 
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));    
      esp_err_t result=esp_now_send(broadcastAddress,(uint8_t *)&pesan,sizeof(pesan));
//    }
//  }
}
