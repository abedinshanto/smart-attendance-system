#ifndef DISPLAY_ENGINE_H
#define DISPLAY_ENGINE_H
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
extern Adafruit_SSD1306 display;
// --- Icons (16x16 bitmaps) ---
// Checkmark
const unsigned char PROGMEM icon_check[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x06, 0x00,
    0x0C, 0x00, 0x18, 0x40, 0x30, 0x60, 0x60, 0x30, 0xC0, 0x19, 0x80,
    0x0F, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// Cross (X)
const unsigned char PROGMEM icon_cross[] = {
    0x00, 0x00, 0xC0, 0x03, 0xE0, 0x07, 0x70, 0x0E, 0x38, 0x1C, 0x1C,
    0x38, 0x0E, 0x70, 0x07, 0xE0, 0x07, 0xE0, 0x0E, 0x70, 0x1C, 0x38,
    0x38, 0x1C, 0x70, 0x0E, 0xE0, 0x07, 0xC0, 0x03, 0x00, 0x00};
enum DisplayState {
  STATE_BOOT,
  STATE_IDLE,
  STATE_SCAN_SUCCESS,
  STATE_SCAN_ERROR,
  STATE_WIFI_CONNECTING
};
DisplayState currentState = STATE_BOOT;
unsigned long lastStateChange = 0;
String statusMsg = "";
String subMsg = "";
void showBoot() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println(F("SMART ATT"));
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.println(F("System v3.1"));
  display.display();
}
void showWiFiConnecting(int retry) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Connecting WiFi..."));
  // Loading bar
  display.drawRect(10, 30, 108, 10, WHITE);
  display.fillRect(12, 32, (retry * 5) % 104, 6, WHITE);
  display.display();
}
void showIdle(DateTime now, String ip) {
  display.clearDisplay();
  // Top Bar: WiFi Icon (Simulated) & IP
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("WiFi: "));
  display.println(ip);
  display.drawLine(0, 10, 128, 10, WHITE);
  // Large Time
  display.setTextSize(2);
  int hr = now.hour();
  int min = now.minute();
  bool pm = hr >= 12;
  if (hr > 12)
    hr -= 12;
  if (hr == 0)
    hr = 12;
  // Center the time roughly
  display.setCursor(15, 20);
  if (hr < 10)
    display.print('0');
  display.print(hr);
  display.print(':');
  if (min < 10)
    display.print('0');
  display.print(min);
  display.setTextSize(1);
  display.print(pm ? " PM" : " AM");
  // Date at bottom
  display.setCursor(20, 45);
  display.print(now.day());
  display.print('/');
  display.print(now.month());
  display.print('/');
  display.print(now.year());
  display.display();
}
void showScanSuccess(String name, String status) {
  display.clearDisplay();
  // Icon
  display.drawBitmap(0, 0, icon_check, 16, 16, WHITE);
  // Status Header
  display.setTextSize(1);
  display.setCursor(20, 4);
  display.println(status);
  display.drawLine(0, 18, 128, 18, WHITE);
  // Name (Large if short, wrapped if long)
  display.setCursor(0, 25);
  if (name.length() > 10)
    display.setTextSize(1);
  else
    display.setTextSize(2);
  display.println(name);
  display.display();
}
void showScanError(String uid) {
  display.clearDisplay();
  display.drawBitmap(0, 0, icon_cross, 16, 16, WHITE);
  display.setTextSize(1);
  display.setCursor(20, 4);
  display.println(F("Access Denied"));
  display.drawLine(0, 18, 128, 18, WHITE);
  display.setCursor(0, 25);
  display.println(F("Unknown Card:"));
  display.setCursor(0, 40);
  display.println(uid);
  display.display();
}
// Main update loop for display (call this in loop)
void updateDisplay(DateTime now, String ip) {
  // If showing a temporary message (Success/Error), check timeout
  if (currentState == STATE_SCAN_SUCCESS || currentState == STATE_SCAN_ERROR) {
    if (millis() - lastStateChange > 3000) { // 3 seconds timeout
      currentState = STATE_IDLE;
    } else {
      return; // Keep showing the message
    }
  }
  if (currentState == STATE_IDLE) {
    showIdle(now, ip);
  }
}
void triggerScanSuccess(String name, String status) {
  currentState = STATE_SCAN_SUCCESS;
  lastStateChange = millis();
  showScanSuccess(name, status);
}
void triggerScanError(String uid) {
  currentState = STATE_SCAN_ERROR;
  lastStateChange = millis();
  showScanError(uid);
}
#endif