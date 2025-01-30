/*
 * Contoh penggunaan library DMDESP
 * 
 * email : bonny@grobak.net - www.grobak.net - www.elektronmart.com  
 * 
 * DejaVuSans9
 * DejaVuSansBold9
 * DejaVuSansItalic9
 * ElektronMart5x6
 * ElektronMart6x8
 * ElektronMart6x12
 * ElektronMart6x16
 * ElektronMartArabic3x6
 * ElektronMartArabic5x6
 * ElektronMartArabic6x16
 * EMKotak5x7
 * EMSans5x6
 * EMSans6x8
 * EMSans6x16
 * EMSans8x16
 * EMSansSP8x16
 * Mono5x7
*/

#include <DMDESP.h>
//#include <fonts/DejaVuSans9.h>
//#include <fonts/DejaVuSansBold9.h>
//#include <fonts/DejaVuSansItalic9.h>
//#include <fonts/ElektronMart5x6.h>
#include <fonts/ElektronMart6x8.h>
#include <fonts/ElektronMart6x12.h>
#include <fonts/ElektronMart6x16.h>
//#include <fonts/ElektronMartArabic3x6.h>
//#include <fonts/ElektronMartArabic5x6.h>
//#include <fonts/ElektronMartArabic6x16.h>
//#include <fonts/EMKotak5x7.h>
//#include <fonts/EMSans5x6.h>
//#include <fonts/EMSans6x8.h>
//#include <fonts/EMSans6x16.h>
//#include <fonts/EMSans8x16.h>
#include <fonts/EMSansSP8x16.h>
//#include <fonts/Mono5x7.h>
#include "ElektronMart6x12.h"
#include "SystemFont5x7.h"
#include "verdanav2.h"

//SETUP DMD
#define DISPLAYS_WIDE 2 // Kolom Panel
#define DISPLAYS_HIGH 1 // Baris Panel
DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);  // Jumlah Panel P10 yang digunakan (KOLOM,BARIS)



//----------------------------------------------------------------------
// SETUP

void setup() {

  // DMDESP Setup
  Disp.start(); // Jalankan library DMDESP
  Disp.setBrightness(15); // Tingkat kecerahan
//  Disp.setFont(EMSansSP8x16); // Tentukan huruf
  
}



//----------------------------------------------------------------------
// LOOP

void loop() {
  Disp.loop(); // Jalankan Disp loop untuk refresh LED
  
  Disp.setFont(ElektronMart6x8);
  Disp.drawText(-2, 0,"#");
  Disp.setFont(verdanav2); 
  Disp.drawText(15, -1," ");
  Disp.drawText(17, -1,"12:20:59");
  Disp.drawRect(0,7,Disp.width(),7,1,1);
  TeksJalan(8, 50);
  
  Disp.drawRect(62,14,63,15,1,1);
//  Disp.setFont(EMSansSP8x16); // Tentukan huruf
//  TeksJalan(-1, 40); // Tampilkan teks berjalan TeksJalan(posisi y, kecepatan);
//  Disp.setFont(SystemFont5x7); // Tentukan huruf
//  
//  Disp.drawText(1, 8, "00");
//  Disp.drawText(20, 8, "49");
//  Disp.drawText(14, 8, ":");
//  delay(14900);
//  Disp.drawText(0,0,"          "); // Tampilkan teks
//  delay(1000);
//
//  Disp.setFont(EMSans5x6); // Tentukan huruf
//  Disp.drawText(0,0,"DMDESP"); // Tampilkan teks
//  TeksJalan(7, 50); // Tampilkan teks berjalan TeksJalan(posisi y, kecepatan);
//  delay(14900);
//  Disp.drawText(0,0,"          "); // Tampilkan teks
//  delay(1000);
}


//  Disp.drawText(0,0,"DMDESP"); // Tampilkan teks
//--------------------------
// TAMPILKAN SCROLLING TEKS

static char *teks[] = {"LURUSKAN DAN RAPATKAN SHAF - MOHON MATIKAN/BISUKAN HANDPHONE"};

void TeksJalan(int y, uint8_t kecepatan) {

  static uint32_t pM;
  static uint32_t x;
  int width = Disp.width();
//  Disp.setFont(DejaVuSansBold9);
  int fullScroll = Disp.textWidth(teks[0]) + width;
  if((millis() - pM) > kecepatan) { 
    pM = millis();
    if (x < fullScroll) {
      ++x;
    } else {
      x = 0;
      return;
    }
    Disp.drawText(width - x, y, teks[0]);
  }  

}
