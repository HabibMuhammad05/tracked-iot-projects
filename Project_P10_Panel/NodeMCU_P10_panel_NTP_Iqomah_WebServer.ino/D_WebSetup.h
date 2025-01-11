// ============================================= EEPROM ======================================
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
  //============= MAIN VAR =============
  T1_SSID  = read_EEPROM(51,100);
  T1_SSID  = T1_SSID.substring(0, T1_SSID.length());
  ssid = T1_SSID.c_str();
  
  T2_PASS  = read_EEPROM(101,150);
  T2_PASS  = T2_PASS.substring(0,T2_PASS.length());
  password = T2_PASS.c_str();
  
  T3_latitude  = read_EEPROM(151,200);
  T3_latitude  = T3_latitude.substring(0, T3_latitude.length());
  lati = atof(T3_latitude.c_str());

  T4_longitude  = read_EEPROM(201,250);
  T4_longitude  = T4_longitude.substring(0,T4_longitude.length());
  longi = atof(T4_longitude.c_str());
  
  T5_Catatan  = read_EEPROM(251,300);
  T5_Catatan  = T5_Catatan.substring(0,T5_Catatan.length());
  PesanTeks = T5_Catatan.c_str();

  Tsholat  = read_EEPROM(316,318);
  Tsholat  = Tsholat.substring(0, Tsholat.length());
  durasiSholat = atoi(Tsholat.c_str());
  
  Ts  = read_EEPROM(301,303);
  Ts  = Ts.substring(0, Ts.length());
  iqmhs = atoi(Ts.c_str());
  Td  = read_EEPROM(304,306);
  Td  = Td.substring(0, Td.length());
  iqmhd = atoi(Td.c_str());
  Ta  = read_EEPROM(307,309);
  Ta  = Ta.substring(0, Ta.length());
  iqmha = atoi(Ta.c_str());
  Tm  = read_EEPROM(310,312);
  Tm  = Tm.substring(0, Tm.length());
  iqmhm = atoi(Tm.c_str());
  Ti  = read_EEPROM(313,315);
  Ti  = Ti.substring(0, Ti.length());
  iqmhi = atoi(Ti.c_str());
  
  DEBUG_PRINT("SSID : "); DEBUG_PRINTLN(ssid);
  DEBUG_PRINT("PASS : "); DEBUG_PRINTLN(password);
  
  DEBUG_PRINT("LAT : "); DEBUG_PRINTLN(lati);
  DEBUG_PRINT("LONG : "); DEBUG_PRINTLN(longi);
  
  DEBUG_PRINT("TEKS : "); DEBUG_PRINTLN(PesanTeks);
  DEBUG_PRINT("Durasi Sholat : "); DEBUG_PRINTLN(durasiSholat);
  DEBUG_PRINT("Iqomah Subuh : "); DEBUG_PRINTLN(iqmhs);
  DEBUG_PRINT("Iqomah Dzuhur : "); DEBUG_PRINTLN(iqmhd);
  DEBUG_PRINT("Iqomah Ashar : "); DEBUG_PRINTLN(iqmha);
  DEBUG_PRINT("Iqomah Maghrib : "); DEBUG_PRINTLN(iqmhm);
  DEBUG_PRINT("Iqomah Isya : "); DEBUG_PRINTLN(iqmhi);

}

// ========================================== WEB Interface ==================================
void handleRoot() {
  String T1 = String(ssid);
  String T2 = String(password);
  String T3 = String(lati, 5);
  String T4 = String(longi, 5);

  String html="<!DOCTYPE html>";
  html=html+"  <html lang='en'>";
  html=html+"  <head>";
  html=html+"  <meta charset='UTF-8'>";
  html=html+"  <meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html=html+"  <title>P10 JWS Setting Page</title>";
  html=html+"  <style>";
  html=html+"  body {";
  html=html+"      font-family: Arial, sans-serif;";
  html=html+"      background-color: lightslategray;";
  html=html+"      display: flex;";
  html=html+"      justify-content: center;";
  html=html+"     align-items: center;";
  html=html+"      height: 140vh;";
  html=html+"      margin: 0;";
  html=html+"  }";
  html=html+"  .container {";
  html=html+"      background-color: gray;";
  html=html+"      padding: 20px;";
  html=html+"      border-radius: 10px;";
  html=html+"      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);";
  html=html+"      max-width: 600px;";
  html=html+"      width: 900%;";
  html=html+"  }";
  html=html+"  h1, h2 {";
  html=html+"      color: #333;";
  html=html+"      text-align: center;";
  html=html+"  }";
  html=html+"  table {";
  html=html+"      width: 100%;";
  html=html+"      border-collapse: collapse;";
  html=html+"      margin-top: 20px;";
  html=html+"  }";
  html=html+"  table, th, td {";
  html=html+"      border: 1px solid #ccc;";
  html=html+"  }";
  html=html+"  th, td {";
  html=html+"      padding: 10px;";
  html=html+"      text-align: center;";
  html=html+"  }";
  html=html+"  input[type='text'] {";
  html=html+"      width: 100%;";
  html=html+"      padding: 8px;";
  html=html+"      margin-bottom: 10px;";
  html=html+"      border: 1px solid #ccc;";
  html=html+"      border-radius: 4px;";
  html=html+"      box-sizing: border-box;";
  html=html+"  }";
  html=html+"  input[type='number'] {";
  html=html+"      border: 1px solid #ccc;";
  html=html+"      border-radius: 4px;";
  html=html+"      box-sizing: border-box;";
  html=html+"  }";
  html=html+"  input[type='submit'] {";
  html=html+"      background-color: #4CAF50;";
  html=html+"      color: #fff;";
  html=html+"      padding: 10px 20px;";
  html=html+"      border: none;";
  html=html+"      border-radius: 4px;";
  html=html+"      cursor: pointer;";
  html=html+"      font-size: 16px;";
  html=html+"      width: 100%;";
  html=html+"  }";
  html=html+"  input[type='submit']:hover {";
  html=html+"      background-color: #45a049;";
  html=html+"  }";
  html=html+"  h5 a {";
  html=html+"      color: #007BFF;";
  html=html+"      text-decoration: none;";
  html=html+"      display: block;";
  html=html+"      text-align: center;";
  html=html+"  }";
  html=html+"  </style>";
  html=html+"  </head>";
  html=html+"  <body>";
  html=html+"  <div class='container'>    ";
  html=html+"      <h1>SETTING MENU</h1> ";
  html=html+"    <h2>P10 Jam Waktu Sholat ESP8266</h3> ";
  html=html+"    <form action='/overwriteSettings' method='get'> ";
  html=html+"        <table>";
  html=html+"                <td colspan='2'> ";
  html=html+"                    <label>Setting WiFi untuk disambungkan</label> ";
  html=html+"                </td> ";
  html=html+"            <tr> ";
  html=html+"                <td style='width:25%' ><b>SSID WiFi</b></td> ";
  html=html+"                <td style='width:75%' ><input type='text' name='text1' value='"+T1+"' maxlength='50'></td> ";
  html=html+"            </tr> ";
  html=html+"            <tr> ";
  html=html+"                <td style='width:25%' ><b>Password WiFi</b></td> ";
  html=html+"                <td style='width:75%' ><input type='text' name='text2' value='"+T2+"' maxlength='50'></td> ";
  html=html+"            </tr> ";
  html=html+"        </table>";
  html=html+"        <table>";
  html=html+"            <tr> ";
  html=html+"                <td colspan='2'> ";
  html=html+"                   <label>Setting lokasi untuk jadwal sholat</label> ";
  html=html+"                </td> ";
  html=html+"            <tr> ";
  html=html+"                <td style='width:25%' ><b>Latitude</b></td> ";
  html=html+"                <td style='width:75%' ><input type='text' name='text3' value='"+T3+"' maxlength='50'></td> ";
  html=html+"            </tr> ";
  html=html+"            <tr> ";
  html=html+"                <td style='width:25%' ><b>Longitude</b></td> ";
  html=html+"                <td style='width:75%' ><input type='text' name='text4' value='"+T4+"' maxlength='50'></td> ";
  html=html+"            </tr> ";
  html=html+"        </table>";
  html=html+"        <table>";
  html=html+"                <td style='width:25%'><b>Teks Catatan:</b></td> ";
  html=html+"                <td style='width:75%'><input type='text' name='text5' value='"+PesanTeks+"' maxlength='50'></td> ";
  html=html+"        </table>";
  html=html+"        <table>";
  html=html+"            <tr> ";
  html=html+"                <td colspan='5'> ";
  html=html+"                    <label>Setting Lama Iqomah dan Sholat (1-30 Menit)</label> ";
  html=html+"                </td> ";
  html=html+"            <tr> ";
  html=html+"                <td colspan='3'><b>Lama Sholat:</b></td> ";
  html=html+"                <td colspan='2'><input type='number' name='sholat' value='"+durasiSholat+"' min='1' max='30'></td> ";
  html=html+"           </tr> ";
  html=html+"            <tr> ";
  html=html+"                <td style='width:20%' ><b>Subuh</b></td> ";
  html=html+"                <td style='width:20%' ><b>Dzuhur</b></td> ";
  html=html+"                <td style='width:20%' ><b>Ashar</b></td> ";
  html=html+"                <td style='width:20%' ><b>Maghrib</b></td> ";
  html=html+"                <td style='width:20%' ><b>Isya</b></td> ";
  html=html+"            </tr> ";
  html=html+"            <tr> ";
  html=html+"                <td><input style='width:80%' type='number' name='s' value='"+iqmhs+"' min='1' max='30'></td> ";
  html=html+"                <td><input style='width:80%' type='number' name='d' value='"+iqmhd+"' min='1' max='30'></td> ";
  html=html+"                <td><input style='width:80%' type='number' name='a' value='"+iqmha+"' min='1' max='30'></td> ";
  html=html+"                <td><input style='width:80%' type='number' name='m' value='"+iqmhm+"' min='1' max='30'></td> ";
  html=html+"                <td><input style='width:80%' type='number' name='i' value='"+iqmhi+"' min='1' max='30'></td> ";
  html=html+"            </tr>  ";
  html=html+"        </table>";
  html=html+"        <table>";
  html=html+"            <tr> ";
  html=html+"                <td colspan='2'> ";
  html=html+"                   <input type='submit' name='Save' value='Submit'>";
  html=html+"                   <h5><a href='http://"+ipAdress+"/update'>© Update JWS P10 NTP</a></h5>";
  html=html+"                   <h5>";
  html=html+"                     <a href='https://wa.me/6285862929263'>Kontak Habib Muhammad</a>";
  html=html+"                   </h5>";
  html=html+"                </td>";
  html=html+"            </tr> ";
  html=html+"        </table>";
  html=html+"      </form>";
  html=html+"  </div>";
  html=html+"  </body>";
  html=html+"  </html>";
  server.send(200,"html",html);
}

void overwriteSettings(){
  //============= MAIN VAR =============
  String text1 = server.arg("text1");
  write_EEPROM(51, 100, text1);
  
  String text2 = server.arg("text2");
  write_EEPROM(101, 150, text2);
  
  String text3 = server.arg("text3");
  write_EEPROM(151, 200, text3);
  
  String text4 = server.arg("text4");
  write_EEPROM(201, 250, text4);
  
  String text5 = server.arg("text5");
  write_EEPROM(251, 300, text5);
  
  String sholat = server.arg("sholat");
  write_EEPROM(316, 318, sholat);
  
  String s = server.arg("s");
  write_EEPROM(301, 303, s);
  String d = server.arg("d");
  write_EEPROM(304, 306, d);
  String a = server.arg("a");
  write_EEPROM(307, 309, a);
  String m = server.arg("m");
  write_EEPROM(310, 312, m);
  String i = server.arg("i");
  write_EEPROM(313, 315, i);
  
  
  String html="<!DOCTYPE html>";
  html=html+"<html lang='en'>";
  html=html+"<head>";
  html=html+"<meta charset='UTF-8'>";
  html=html+"<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html=html+"<title>Setting Berhasil!</title>";
  html=html+"<style>";
  html=html+"body {";
  html=html+"    font-family: Arial, sans-serif;";
  html=html+"    text-align: center;";
  html=html+"    background-color: #f4f4f4;";
  html=html+"    display: flex;";
  html=html+"    justify-content: center;";
  html=html+"    align-items: center;";
  html=html+"    height: 140vh;";
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
  html=html+"    <h1>Setting Done</h1>";
  html=html+"    <h2>SSID WiFi : "+text1+"</h2>";
  html=html+"    <h2>Pass WiFi : "+text2+"</h2>";
  html=html+"    <h2>Latitude  : "+text3+"</h2>";
  html=html+"    <h2>Longitude : "+text4+"</h2>";
  html=html+"    <h2>Catatan   : "+text5+"</h2>";
  html=html+"    <h1>Set Variabel</h1>";
  html=html+"    <h2>Lama Sholat   : "+sholat+"</h2>";
  html=html+"    <h2>Iqomah Subuh  : "+s+"</h2>";
  html=html+"    <h2>Iqomah Dzuhur : "+d+"</h2>";
  html=html+"    <h2>Iqomah Ashar  : "+a+"</h2>";
  html=html+"    <h2>Iqomah Maghrib: "+m+"</h2>";
  html=html+"    <h2>Iqomah Isya   : "+i+"</h2>";
  html=html+"</div>";
  html=html+"</body>";
  html=html+"</html>";
  server.send(200, "text/html", html);  

  reload_EEPROM();

  digitalWrite(buzzer,HIGH); delay(500); digitalWrite(buzzer,LOW);
  digitalWrite(buzzer,HIGH); delay(500); digitalWrite(buzzer,LOW);
  DEBUG_PRINTLN("Setup done, resetting");
  delay(2000);
  ESP.restart();
//  bInMessages = true;
}
