//=============================== NTP CALCULATION ===============================//
time_t requestSync(){
  Serial.write(7); 
  return 0; 
}

//============================== NTP DATA RETRIEVE ===============================//
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
unsigned long Epoch_Time; 

void getTime(){ 
  timeClient.update();
  Epoch_Time = timeClient.getEpochTime()+ 1;
  PRINT("Epoch: "); PRINTLN(Epoch_Time);
  if( Epoch_Time >= 1735693200 and Epoch_Time <= 2000000000) { 
     setTime(Epoch_Time);
  }else{
    PRINTLN(" Date Failed, Resetting...");
    ESP.restart();
  }
}

void updateTime(){
  h = hour();
  m = minute();
  s = second();
  NTPday = day();
  NTPdayOfWeek = weekday();
  NTPmonth = month();
  NTPyear = year();
}
