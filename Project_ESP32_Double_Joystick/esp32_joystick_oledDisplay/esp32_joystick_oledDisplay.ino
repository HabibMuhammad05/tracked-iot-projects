#include <Arduino.h>
#include <U8g2lib.h>

// Initialize the display with the specified pins
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 22, /* data=*/ 23, /* cs=*/ U8X8_PIN_NONE, /* dc=*/ 21, /* reset=*/ 19);

// Define joystick pins
const int joy1XPin = 36; // Joystick 1 X-axis
const int joy1YPin = 39; // Joystick 1 Y-axis
const int joy2XPin = 34; // Joystick 2 X-axis
const int joy2YPin = 35; // Joystick 2 Y-axis

// Define box dimensions
const int boxSize = 32;
const int joy1XBox = 52;
const int joy1YBox = 45;
const int joy2XBox = 84;
const int joy2YBox = 45;

// Calibration variables
int joy1XCenter = 0;
int joy1YCenter = 0;
int joy2XCenter = 0;
int joy2YCenter = 0;

// Threshold value to create a dead zone around the center
const int centerThreshold = 5;

void setup() {
  u8g2.begin();
  Serial.begin(115200);

  // Calibration phase
  calibrateJoysticks();
}

void loop() {
  // Read joystick positions
  int joy1XValue = analogRead(joy1XPin) - joy1XCenter;
  int joy1YValue = analogRead(joy1YPin) - joy1YCenter;
  int joy2XValue = analogRead(joy2XPin) - joy2XCenter;
  int joy2YValue = analogRead(joy2YPin) - joy2YCenter;

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

  // Clear the display
  u8g2.clearBuffer();

  // Draw boxes
  u8g2.drawFrame(joy1XBox - boxSize / 2, joy1YBox - boxSize / 2, boxSize, boxSize);
  u8g2.drawFrame(joy2XBox - boxSize / 2, joy2YBox - boxSize / 2, boxSize, boxSize);

  // Draw joystick positions
  u8g2.drawDisc(joy1XPos, joy1YPos, 2, U8G2_DRAW_ALL);
  u8g2.drawDisc(joy2XPos, joy2YPos, 2, U8G2_DRAW_ALL);

  // Send buffer to display
  u8g2.sendBuffer();

  // Add a small delay to slow down the update rate
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
