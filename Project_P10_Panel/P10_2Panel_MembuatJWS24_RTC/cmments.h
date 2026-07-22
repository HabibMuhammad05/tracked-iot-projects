
//void TampilJamKecil() {
//  static uint32_t pMJam;
//  uint32_t cM = millis();
//  
//  char jam[9];
//  
//  if (cM - pMJam >= 1000) {
//   
//    pMJam = cM;
//    
//    //JAM
//    sprintf(jam,"%02d:%02d:%02d", rJam, rMen, rDet);
//    Disp.setFont(ElektronMart5x6);
//    Disp.drawText(20,7, jam);
//        
//  }
// 
//}

//void TeksBerjalanKiriKanan() {
//
//  static uint32_t pM;
//  uint32_t cM = millis();
//  static uint8_t Kecepatan = 50;
//  static uint32_t x;
//
//  int width = Disp.width();
//  int fullScroll = Disp.textWidth(configjws.namamasjid) + width;
//
//  Disp.setFont(ElektronMart6x8);
//
//  if(cM - pM > Kecepatan) {
//
//    pM = cM;
//
//    if (x < fullScroll) {
//
//      ++x;
//      
//    } else {
//
//      x = 0;
//      
//    }
//
//    Disp.drawText(x - fullScroll + width, 8, configjws.namamasjid);
//    
//  }
//  
//}

//----------------------------------------------------------------------
//TAMPILKAN TANGGAL HIJRIAH

//void TampilTanggalHijr() {
//
//  char hari[8];
//  char tanggal[7]; // 19 ROM
//
//  static uint32_t pM;
//  uint32_t cM = millis();
//
//  static uint8_t flag;
//
//  if(cM - pM > 3000) {
//    
//    pM = cM;
//    flag++;
//
//    sprintf(hari, "%s", hariHijriah[rHar]);
//    sprintf(tanggal, "%02d %s", Hjr_Date, bulanHijriah[Hjr_Month]);
//  
//    Disp.setFont(ElektronMart5x6);
//    textCenter1PKanan(0, hari);
//    textCenter1PKanan(8, tanggal);
//
//    if (flag >= 2) {
//      flag = 0;
//      Disp.drawRect(32,0,64,15,0,0);
//      tmpinfo = 2;
//      
//    }
//    
//  } 
//  
//}


//----------------------------------------------------------------------
// TAMPILKAN SUHU

//void TampilSuhu() {
//
//  char suhu[3];
//  int koreksisuhu = 2; // Perkiraan selisih suhu mesin dengan suhu ruangan
//
//  static uint32_t pM;
//  uint32_t cM = millis();
//
//  static uint8_t flag;
//
//  DEBUG_PRINTLN("MASUKtAMPILsUHU");
//  if(cM - pM > 3000) {
//    
//    pM = cM;
//    flag++;
//
//    sprintf(suhu, "%02d*C", celsius - koreksisuhu);
//    DEBUG_PRINTLN("STRING SUHU DIBUAT");
//    Disp.setFont(ElektronMart5x6);
//    textCenter1PKanan(0, "SUHU");
//    textCenter1PKanan(8, suhu);
//
//    if (flag >= 2) {
//    DEBUG_PRINTLN("STRING SUHU DIBUAT");
//      flag = 0;
//      Disp.drawRect(32,0,64,15,0,0);
//      tmpinfo = 1;
//      
//    }
//  
//  }
//  
//}


//      TampilTanggalHijr();
//      tmpinfo = 2;
