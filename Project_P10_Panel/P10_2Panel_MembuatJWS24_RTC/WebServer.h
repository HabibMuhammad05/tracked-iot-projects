void WifiShutdown() {
  static bool cekWifi = false;

  if (!cekWifi && millis() >= 3600000) {
    WiFi.mode(WIFI_OFF);   
    cekWifi = true;  
//    removeSubstring(configjws.namamasjid, ipBuffer);
    DEBUG_PRINTLN("WiFi off after 1 hour.");
    digitalWrite(buzzer,HIGH); delay(80);  digitalWrite(buzzer,LOW);
  }
}
// WIFI

// LED Internal
uint8_t pin_led = 2;

//WEB Server
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

// Sebagai Station
const char* wifissid = "ANANG -2.4G"; //kalau gagal konek
const char* wifipassword = "umikalsum41169";

// Sebagai AccessPoint
const char* ssid = "JWSP10"; //kalau gagal konek
const char* password = "elektronmart";

IPAddress local_ip(4, 3, 2, 1);
IPAddress gateway(4, 3, 2, 1);
IPAddress netmask(255, 255, 255, 0);



void wifiConnect() {

  WiFi.softAPdisconnect(true);
  WiFi.disconnect();
  delay(1000);

  DEBUG_PRINTLN("Mencoba sambungan ke Hotspot atau Router");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifissid, wifipassword);
  
  unsigned long startTime = millis();
  
  while (WiFi.status() != WL_CONNECTED) {
    
    delay(500);
    DEBUG_PRINT(".");
    
    digitalWrite(pin_led, !digitalRead(pin_led));
    
    if (millis() - startTime > 3000) {
      DEBUG_PRINTLN(" ");
      break;
    }
    
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    
    digitalWrite(pin_led, HIGH);
    DEBUG_PRINT("MAC: ");
    DEBUG_PRINTLN(WiFi.macAddress());
    DEBUG_PRINT("IP: ");
    DEBUG_PRINTLN(WiFi.localIP());
    
  } else {
    
    DEBUG_PRINTLN("Gagal tersambung ke Hotspot, mode Hotspot aktif.");
    
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, netmask);
    WiFi.softAP(ssid, password);
    
    digitalWrite(pin_led, HIGH);

    DEBUG_PRINT("MAC: ");
    DEBUG_PRINTLN(WiFi.macAddress());
    DEBUG_PRINT("IP: ");
    DEBUG_PRINTLN(local_ip);
    
  }

}



void handleXMLWaktu(){
  XMLWaktu();
  server.send(200,"text/xml",XML);
}

void handleXMLDataJWS(){
  XMLDataJWS();
  server.send(200,"text/xml",XML);
}


void handleSettingJwsUpdate() {

  String datajws = server.arg("plain");

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, datajws);

  File configFileJws = LittleFS.open(fileconfigjws, "w");
  
  if (!configFileJws) {
    DEBUG_PRINTLN("Gagal membuka Info configFileJws untuk ditulis");
    return;
  }
  
  serializeJson(doc, configFileJws);

  if (error) {
    
    DEBUG_PRINT(F("deserializeJson() gagal kode sebagai berikut: "));
    DEBUG_PRINTLN(error.c_str());
    return;
    
  } else {
    
    configFileJws.close();
    DEBUG_PRINTLN("Berhasil mengubah configFileJws");

    loadJwsConfig(fileconfigjws, configjws);
  
  }  

}
