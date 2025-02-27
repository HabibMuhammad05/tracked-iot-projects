 /**********************************************************************************
 *  TITLE: Blynk 2.0 + IR + Manual Switch control 8 Relays using ESP32 with DHT11 sensor, LDR, Realtime Feedback & No WiFi control
 *  Click on the following links to learn more. 
 *  YouTube Video: https://youtu.be/Vgyj3cCAioI
 *  Related Blog : https://iotcircuithub.com/esp32-projects/
 *  by Tech StudyCell
 *  Preferences--> Aditional boards Manager URLs : 
 *  https://dl.espressif.com/dl/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *  
 *  Download Board ESP32 (1.0.6) : https://github.com/espressif/arduino-esp32
 *  
 *  Download the Libraries:
 *  Blynk 1.0.1 Library:  https://github.com/blynkkk/blynk-library
 *  IRremote Library (3.3.0): https://github.com/Arduino-IRremote/Arduino-IRremote
 *  DHT Library (1.4.2): https://github.com/adafruit/DHT-sensor-library
 **********************************************************************************/

// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "iMPMMbE-6sy"
#define BLYNK_DEVICE_NAME "espku"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
//#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_WROVER_BOARD

#include <DHT.h>  
#include <IRremote.h>

#define IR_RECV_PIN 35 // D35 (IR receiver pin)

IRrecv irrecv(IR_RECV_PIN);
decode_results results;
  
// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301

#define DHTPIN              16 //D16  pin connected with DHT
#define LDR_PIN             34 //D34  pin connected with LDR

// define the GPIO connected with Relays and switches
#define RelayPin1 23  //D23
#define RelayPin2 22  //D22
#define RelayPin3 21  //D21
#define RelayPin4 19  //D19
#define RelayPin5 18  //D18
#define RelayPin6 5   //D5
#define RelayPin7 25  //D25
#define RelayPin8 26  //D26

#define SwitchPin1 13  //D13
#define SwitchPin2 12  //D12
#define SwitchPin3 14  //D14
#define SwitchPin4 27  //D27
#define SwitchPin5 33  //D33
#define SwitchPin6 32  //D32
#define SwitchPin7 15  //D15
#define SwitchPin8 4   //D4

#define wifiLed    2   //D2

#define VPIN_BUTTON_1    V1 
#define VPIN_BUTTON_2    V2
#define VPIN_BUTTON_3    V3 
#define VPIN_BUTTON_4    V4
#define VPIN_BUTTON_5    V5 
#define VPIN_BUTTON_6    V6
#define VPIN_BUTTON_7    V7 
#define VPIN_BUTTON_8    V8

#define VPIN_BUTTON_C    V9
#define VPIN_TEMPERATURE V10
#define VPIN_HUMIDITY    V11
#define VPIN_LDR         V12

// Relay State
bool toggleState_1 = LOW; //Define integer to remember the toggle state for relay 1
bool toggleState_2 = LOW; //Define integer to remember the toggle state for relay 2
bool toggleState_3 = LOW; //Define integer to remember the toggle state for relay 3
bool toggleState_4 = LOW; //Define integer to remember the toggle state for relay 4
bool toggleState_5 = LOW; //Define integer to remember the toggle state for relay 5
bool toggleState_6 = LOW; //Define integer to remember the toggle state for relay 6
bool toggleState_7 = LOW; //Define integer to remember the toggle state for relay 7
bool toggleState_8 = LOW; //Define integer to remember the toggle state for relay 8

// Switch State
bool SwitchState_1 = LOW;
bool SwitchState_2 = LOW;
bool SwitchState_3 = LOW;
bool SwitchState_4 = LOW;
bool SwitchState_5 = LOW;
bool SwitchState_6 = LOW;
bool SwitchState_7 = LOW;
bool SwitchState_8 = LOW;

float temperature1 = 0;
float humidity1   = 0;
int   ldrVal;

#include "BlynkEdgent.h"

BlynkTimer timer2;

DHT dht(DHTPIN, DHTTYPE);



void setup()
{
  Serial.begin(115200);

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);
  pinMode(RelayPin4, OUTPUT);
  pinMode(RelayPin5, OUTPUT);
  pinMode(RelayPin6, OUTPUT);
  pinMode(RelayPin7, OUTPUT);
  pinMode(RelayPin8, OUTPUT);

  pinMode(wifiLed, OUTPUT);

  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);
  pinMode(SwitchPin3, INPUT_PULLUP);
  pinMode(SwitchPin4, INPUT_PULLUP);
  pinMode(SwitchPin5, INPUT_PULLUP);
  pinMode(SwitchPin6, INPUT_PULLUP);
  pinMode(SwitchPin7, INPUT_PULLUP);
  pinMode(SwitchPin8, INPUT_PULLUP);
  
  //During Starting all Relays should TURN OFF
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);
  digitalWrite(RelayPin3, HIGH);
  digitalWrite(RelayPin4, HIGH);
  digitalWrite(RelayPin5, HIGH);
  digitalWrite(RelayPin6, HIGH);
  digitalWrite(RelayPin7, HIGH);
  digitalWrite(RelayPin8, HIGH);

  irrecv.enableIRIn(); // Enabling IR sensor
  BlynkEdgent.begin();
  dht.begin();
  
  // Setup a function to be called every 2 seconds
  timer2.setInterval(2000L, sendSensor);

}
void loop() {

    BlynkEdgent.run();
    
    manual_control(); //Manual Switch Control
    ir_remote(); //IR remote Control
    timer2.run();
}

void readSensor(){
    
  ldrVal = map(analogRead(LDR_PIN), 0, 4095, 10, 0);
  
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  else {
    humidity1 = h;
    temperature1 = t;
   // Serial.println(temperature1);
   // Serial.println(ldrVal);
  }  
}

void sendSensor()
{
  readSensor();
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(VPIN_HUMIDITY, humidity1);
  Blynk.virtualWrite(VPIN_TEMPERATURE, temperature1);
  Blynk.virtualWrite(VPIN_LDR, ldrVal);
}

//BLYNK_CONNECTED() {
//  // Request the latest state from the server
//  Blynk.syncVirtual(VPIN_BUTTON_1);
//  Blynk.syncVirtual(VPIN_BUTTON_2);
//  Blynk.syncVirtual(VPIN_BUTTON_3);
//  Blynk.syncVirtual(VPIN_BUTTON_4);
//  Blynk.syncVirtual(VPIN_BUTTON_5);
//  Blynk.syncVirtual(VPIN_BUTTON_6);
//  Blynk.syncVirtual(VPIN_BUTTON_7);
//  Blynk.syncVirtual(VPIN_BUTTON_8);
//  Blynk.syncVirtual(VPIN_TEMPERATURE);
//  Blynk.syncVirtual(VPIN_HUMIDITY);
//  Blynk.syncVirtual(VPIN_LDR);
//}

// When App button is pushed - switch the state

BLYNK_WRITE(VPIN_BUTTON_1) {
  toggleState_1 = param.asInt();
  if(toggleState_1 == 1){
    digitalWrite(RelayPin1, LOW);
  }
  else { 
    digitalWrite(RelayPin1, HIGH);
  }
}

BLYNK_WRITE(VPIN_BUTTON_2) {
  toggleState_2 = param.asInt();
  if(toggleState_2 == 1){
    digitalWrite(RelayPin2, LOW);
  }
  else { 
    digitalWrite(RelayPin2, HIGH);
  }
}

BLYNK_WRITE(VPIN_BUTTON_3) {
  toggleState_3 = param.asInt();
  if(toggleState_3 == 1){
    digitalWrite(RelayPin3, LOW);
  }
  else { 
    digitalWrite(RelayPin3, HIGH);
  }
}

BLYNK_WRITE(VPIN_BUTTON_4) {
  toggleState_4 = param.asInt();
  if(toggleState_4 == 1){
    digitalWrite(RelayPin4, LOW);
  }
  else { 
    digitalWrite(RelayPin4, HIGH);
  }
}

BLYNK_WRITE(VPIN_BUTTON_5) {
  toggleState_5 = param.asInt();
  if(toggleState_5 == 1){
    digitalWrite(RelayPin5, LOW);
  }
  else { 
    digitalWrite(RelayPin5, HIGH);
  }
}

BLYNK_WRITE(VPIN_BUTTON_6) {
  toggleState_6 = param.asInt();
  if(toggleState_6 == 1){
    digitalWrite(RelayPin6, LOW);
  }
  else { 
    digitalWrite(RelayPin6, HIGH);
  }
}

BLYNK_WRITE(VPIN_BUTTON_7) {
  toggleState_7 = param.asInt();
  if(toggleState_7 == 1){
    digitalWrite(RelayPin7, LOW);
  }
  else { 
    digitalWrite(RelayPin7, HIGH);
  }
}

BLYNK_WRITE(VPIN_BUTTON_8) {
  toggleState_8 = param.asInt();
  if(toggleState_8 == 1){
    digitalWrite(RelayPin8, LOW);
  }
  else { 
    digitalWrite(RelayPin8, HIGH);
  }
}

BLYNK_WRITE(VPIN_BUTTON_C) {
  all_SwitchOff();
}


void all_SwitchOff(){
  toggleState_1 = 0; digitalWrite(RelayPin1, HIGH); Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1); delay(100);
  toggleState_2 = 0; digitalWrite(RelayPin2, HIGH); Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2); delay(100);
  toggleState_3 = 0; digitalWrite(RelayPin3, HIGH); Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3); delay(100);
  toggleState_4 = 0; digitalWrite(RelayPin4, HIGH); Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_4); delay(100);
  toggleState_5 = 0; digitalWrite(RelayPin5, HIGH); Blynk.virtualWrite(VPIN_BUTTON_5, toggleState_5); delay(100);
  toggleState_6 = 0; digitalWrite(RelayPin6, HIGH); Blynk.virtualWrite(VPIN_BUTTON_6, toggleState_6); delay(100);
  toggleState_7 = 0; digitalWrite(RelayPin7, HIGH); Blynk.virtualWrite(VPIN_BUTTON_7, toggleState_7); delay(100);
  toggleState_8 = 0; digitalWrite(RelayPin8, HIGH); Blynk.virtualWrite(VPIN_BUTTON_8, toggleState_8); delay(100);
  Blynk.virtualWrite(VPIN_HUMIDITY, humidity1);
  Blynk.virtualWrite(VPIN_TEMPERATURE, temperature1);
  Blynk.virtualWrite(VPIN_LDR, ldrVal);
}

void manual_control()
{
  if (digitalRead(SwitchPin1) == LOW && SwitchState_1 == LOW) {
    digitalWrite(RelayPin1, LOW);
    Blynk.virtualWrite(VPIN_BUTTON_1, HIGH);
    toggleState_1 = HIGH;
    SwitchState_1 = HIGH;
    Serial.println("Switch-1 on");
  }
  if (digitalRead(SwitchPin1) == HIGH && SwitchState_1 == HIGH) {
    digitalWrite(RelayPin1, HIGH);
    Blynk.virtualWrite(VPIN_BUTTON_1, LOW);
    toggleState_1 = LOW;
    SwitchState_1 = 0;
    Serial.println("Switch-1 off");
  }
  if (digitalRead(SwitchPin2) == LOW && SwitchState_2 == LOW) {
    digitalWrite(RelayPin2, LOW);
    Blynk.virtualWrite(VPIN_BUTTON_2, HIGH);
    toggleState_2 = HIGH;
    SwitchState_2 = HIGH;
    Serial.println("Switch-2 on");
  }
  if (digitalRead(SwitchPin2) == HIGH && SwitchState_2 == HIGH) {
    digitalWrite(RelayPin2, HIGH);
    Blynk.virtualWrite(VPIN_BUTTON_2, LOW);
    toggleState_2 = LOW;
    SwitchState_2 = LOW;
    Serial.println("Switch-2 off");
  }
  if (digitalRead(SwitchPin3) == LOW && SwitchState_3 == LOW) {
    digitalWrite(RelayPin3, LOW);
    Blynk.virtualWrite(VPIN_BUTTON_3, HIGH);
    toggleState_3 = HIGH;
    SwitchState_3 = HIGH;
    Serial.println("Switch-3 on");
  }
  if (digitalRead(SwitchPin3) == HIGH && SwitchState_3 == HIGH) {
    digitalWrite(RelayPin3, HIGH);
    Blynk.virtualWrite(VPIN_BUTTON_3, LOW);
    toggleState_3 = LOW;
    SwitchState_3 = LOW;
    Serial.println("Switch-3 off");
  }
  if (digitalRead(SwitchPin4) == LOW && SwitchState_4 == LOW) {
    digitalWrite(RelayPin4, LOW);
    Blynk.virtualWrite(VPIN_BUTTON_4, HIGH);
    toggleState_4 = HIGH;
    SwitchState_4 = HIGH;
    Serial.println("Switch-4 on");
  }
  if (digitalRead(SwitchPin4) == HIGH && SwitchState_4 == HIGH) {
    digitalWrite(RelayPin4, HIGH);
    Blynk.virtualWrite(VPIN_BUTTON_4, LOW);
    toggleState_4 = LOW;
    SwitchState_4 = LOW;
    Serial.println("Switch-4 off");
  }
  if (digitalRead(SwitchPin5) == LOW && SwitchState_5 == LOW) {
    digitalWrite(RelayPin5, LOW);
    Blynk.virtualWrite(VPIN_BUTTON_5, HIGH);
    toggleState_5 = HIGH;
    SwitchState_5 = HIGH;
    Serial.println("Switch-5 on");
  }
  if (digitalRead(SwitchPin5) == HIGH && SwitchState_5 == HIGH) {
    digitalWrite(RelayPin5, HIGH);
    Blynk.virtualWrite(VPIN_BUTTON_5, LOW);
    toggleState_5 = LOW;
    SwitchState_5 = LOW;
    Serial.println("Switch-5 off");
  }
  if (digitalRead(SwitchPin6) == LOW && SwitchState_6 == LOW) {
    digitalWrite(RelayPin6, LOW);
    Blynk.virtualWrite(VPIN_BUTTON_6, HIGH);
    toggleState_6 = HIGH;
    SwitchState_6 = HIGH;
    Serial.println("Switch-6 on");
  }
  if (digitalRead(SwitchPin6) == HIGH && SwitchState_6 == HIGH) {
    digitalWrite(RelayPin6, HIGH);
    Blynk.virtualWrite(VPIN_BUTTON_6, LOW);
    toggleState_6 = LOW;
    SwitchState_6 = LOW;
    Serial.println("Switch-6 off");
  }
  if (digitalRead(SwitchPin7) == LOW && SwitchState_7 == LOW) {
    digitalWrite(RelayPin7, LOW);
    Blynk.virtualWrite(VPIN_BUTTON_7, HIGH);
    toggleState_7 = HIGH;
    SwitchState_7 = HIGH;
    Serial.println("Switch-7 on");
  }
  if (digitalRead(SwitchPin7) == HIGH && SwitchState_7 == HIGH) {
    digitalWrite(RelayPin7, HIGH);
    Blynk.virtualWrite(VPIN_BUTTON_7, LOW);
    toggleState_7 = LOW;
    SwitchState_7 = LOW;
    Serial.println("Switch-7 off");
  }
  if (digitalRead(SwitchPin8) == LOW && SwitchState_8 == LOW) {
    digitalWrite(RelayPin8, LOW);
    Blynk.virtualWrite(VPIN_BUTTON_8, HIGH);
    toggleState_8 = HIGH;
    SwitchState_8 = HIGH;
    Serial.println("Switch-8 on");
  }
  if (digitalRead(SwitchPin8) == HIGH && SwitchState_8 == HIGH) {
    digitalWrite(RelayPin8, HIGH);
    Blynk.virtualWrite(VPIN_BUTTON_8, LOW);
    toggleState_8 = LOW;
    SwitchState_8 = LOW;
    Serial.println("Switch-8 off");
  }
}

void ir_remote(){
  if (irrecv.decode(&results)) {
      switch(results.value){
          case 0x80BF49B6:  
            digitalWrite(RelayPin1, toggleState_1);
            toggleState_1 = !toggleState_1;
            Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
            delay(100);            
            break;
          case 0x80BFC936:  
            digitalWrite(RelayPin2, toggleState_2);
            toggleState_2 = !toggleState_2;
            Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
            delay(100);            
            break;
          case 0x80BF33CC:  
            digitalWrite(RelayPin3, toggleState_3);
            toggleState_3 = !toggleState_3;
            Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3);
            delay(100);            
            break;
          case 0x80BF718E:  
            digitalWrite(RelayPin4, toggleState_4);
            toggleState_4 = !toggleState_4;
            Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_4);
            delay(100);            
            break;
          case 0x80BFF10E:  
            digitalWrite(RelayPin5, toggleState_5);
            toggleState_5 = !toggleState_5;
            Blynk.virtualWrite(VPIN_BUTTON_5, toggleState_5);
            delay(100);            
            break;
          case 0x80BF13EC:  
            digitalWrite(RelayPin6, toggleState_6);
            toggleState_6 = !toggleState_6;
            Blynk.virtualWrite(VPIN_BUTTON_6, toggleState_6);
            delay(100);            
            break;
          case 0x80BF51AE:  
            digitalWrite(RelayPin7, toggleState_7);
            toggleState_7 = !toggleState_7;
            Blynk.virtualWrite(VPIN_BUTTON_7, toggleState_7);
            delay(100);            
            break;
          case 0x80BFD12E:  
            digitalWrite(RelayPin8, toggleState_8);
            toggleState_8 = !toggleState_8;
            Blynk.virtualWrite(VPIN_BUTTON_8, toggleState_8);
            delay(100);            
            break;  
          default : break;         
        }   
        //Serial.println(results.value, HEX);    
        irrecv.resume();   
  } 
}
