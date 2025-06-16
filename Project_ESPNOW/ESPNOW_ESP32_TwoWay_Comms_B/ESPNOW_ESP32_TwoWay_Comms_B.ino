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

A_to_B dataIn;
B_to_A dataOut;

uint8_t broadcastAddress[] = {0x08, 0xB6, 0x1F, 0x4F, 0x12, 0xAC}; // <-- UPDATE THIS

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&dataIn, incomingData, sizeof(dataIn));
  Serial.println("Received from A:");
  Serial.print("  RPM    : "); Serial.println(dataIn.rpm);
  Serial.print("  Voltage: "); Serial.println(dataIn.voltage);
  Serial.print("  MotorOn: "); Serial.println(dataIn.motorOn ? "Yes" : "No");

  // Respond with a different struct
  dataOut.temperature = 36.5;
  dataOut.status = 1;

  esp_now_send(mac, (uint8_t *)&dataOut, sizeof(dataOut));
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println("ESP32 B MAC: " + WiFi.macAddress());

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
  // ESP32 B sends only after receiving
}
