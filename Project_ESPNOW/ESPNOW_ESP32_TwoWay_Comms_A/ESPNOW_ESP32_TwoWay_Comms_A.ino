#include <WiFi.h>
#include <esp_now.h>

struct A_to_B {
  int rpm;
  float voltage;
  bool motorOn;
};

struct B_to_A {
  float temperature;
  uint8_t status;
};

A_to_B dataOut;
B_to_A dataIn;

uint8_t broadcastAddress[] = {0x14, 0x33, 0x5C, 0x02, 0x49, 0x58}; // <-- UPDATE THIS

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&dataIn, incomingData, sizeof(dataIn));
  Serial.println("Received from B:");
  Serial.print("  Temperature: "); Serial.println(dataIn.temperature);
  Serial.print("  Status    : "); Serial.println(dataIn.status);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println("ESP32 A MAC: " + WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop() {
  dataOut.rpm = random(1000, 2000);
  dataOut.voltage = 12.3;
  dataOut.motorOn = true;

  esp_now_send(broadcastAddress, (uint8_t *)&dataOut, sizeof(dataOut));
  Serial.println("Sent to B");
  delay(2000);
}
