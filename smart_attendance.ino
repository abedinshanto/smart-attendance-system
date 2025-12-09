/*
 * Smart Attendance System - Advanced Firmware v3.2
 */

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <MFRC522.h>
#include <RTClib.h>
#include <SPI.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <set>

#include "data_models.h"
#include "display_engine.h" // New Display Engine
#include "time_engine.h"
#include "web_pages.h"

// --- Configuration ---
const char *ssid = "LawL";
const char *password = "shanto4860";

#define BUZZER_PIN 15
#define SS_PIN 5
#define RST_PIN 4

WebServer server(80);
RTC_DS3231 rtc;
MFRC522 mfrc522(SS_PIN, RST_PIN);
// Display object is declared extern in display_engine.h, defined here
Adafruit_SSD1306 display(128, 64, &Wire, -1);

String lastScannedUID = "";
String lastScannedName = "";
String lastScannedTime = "";
int totalPresentToday = 0;

// --- Helper Functions ---

void logEvent(Student s, AttendanceStatus status) {
  File file = LittleFS.open("/attendance.csv", "a");
  if (file) {
    DateTime now = rtc.now();
    // CSV Format: Timestamp,UID,Name,Dept,Sem,Status,Contact
    file.print(now.timestamp());
    file.print(",");
    file.print(s.uid);
    file.print(",");
    file.print(s.name);
    file.print(",");
    file.print(s.dept);
    file.print(",");
    file.print(s.sem);
    file.print(",");
    file.print(getStatusString(status));
    file.print(",");
    file.println(s.contact); // Add Contact Number
    file.close();
  }
}

// --- Global Cache for Duplicates ---
std::set<String> scannedTodayUIDs;

// Update countPresentToday to populate our global set
void countPresentToday() {
  scannedTodayUIDs.clear(); // Reset cache

  File file = LittleFS.open("/attendance.csv", "r");
  if (!file) {
    totalPresentToday = 0;
    return;
  }

  DateTime now = rtc.now();
  String today = now.timestamp().substring(0, 10);

  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.startsWith(today)) {
      if (line.indexOf("Present") > 0 || line.indexOf("Late") > 0) {
        int c1 = line.indexOf(',');
        int c2 = line.indexOf(',', c1 + 1);
        if (c1 > 0 && c2 > c1) {
          String uid = line.substring(c1 + 1, c2);
          scannedTodayUIDs.insert(uid);
        }
      }
    }
  }
  file.close();
  totalPresentToday = scannedTodayUIDs.size();
}

// --- API Handlers ---

void handleRoot() { server.send(200, "text/html", index_html); }

void handleStatus() {
  DateTime now = rtc.now();
  String json = "{";
  json += "\"time\":\"" + formatTime12h(now) + "\",";
  json += "\"lastUid\":\"" + lastScannedUID + "\",";
  json += "\"lastName\":\"" + lastScannedName + "\",";
  json += "\"lastTime\":\"" + lastScannedTime + "\",";
  json += "\"totalPresent\":" + String(totalPresentToday);
  json += "}";
  server.send(200, "application/json", json);
}

void handleGetStudents() {
  server.send(200, "application/json", getAllStudentsJSON());
}

void handleSaveStudent() {
  if (server.hasArg("uid")) {
    Student s;
    s.uid = server.arg("uid");
    s.name = server.arg("name");
    s.dept = server.arg("dept");
    s.sem = server.arg("sem");
    s.roll = server.arg("roll");
    s.reg = server.arg("reg");
    s.contact = server.arg("contact");
    s.blood = server.arg("blood");
    saveStudent(s);
    server.send(200, "text/plain", "OK");
  } else
    server.send(400, "text/plain", "Bad Request");
}

void handleDeleteStudent() {
  if (server.hasArg("uid")) {
    deleteStudent(server.arg("uid"));
    server.send(200, "text/plain", "Deleted");
  } else
    server.send(400, "text/plain", "Missing UID");
}

void handleGetRoutines() {
  File file = LittleFS.open("/routines.json", "r");
  if (!file) {
    server.send(200, "application/json", "{}");
    return;
  }
  server.streamFile(file, "application/json");
  file.close();
}

void handleSaveRoutine() {
  if (server.hasArg("dept") && server.hasArg("sem") && server.hasArg("json")) {
    saveRoutineJSON(server.arg("dept"), server.arg("sem"), server.arg("json"));
    server.send(200, "text/plain", "OK");
  } else
    server.send(400, "text/plain", "Bad Request");
}

void handleResetRoutines() {
  File file = LittleFS.open("/routines.json", "w");
  if (file) {
    file.print("{}");
    file.close();
    server.send(200, "text/plain", "Routines Reset");
  } else {
    server.send(500, "text/plain", "Write Error");
  }
}

// Stream logic to avoid Memory Crashes with large CSVs
void handleGetLogs() {
  String dateFilter = server.arg("date"); // "YYYY-MM-DD"

  if (dateFilter.length() < 10) {
    DateTime now = rtc.now();
    dateFilter = now.timestamp().substring(0, 10);
  }

  File file = LittleFS.open("/attendance.csv", "r");
  if (!file) {
    server.send(200, "application/json", "[]");
    return;
  }

  // Use Chunked Transfer Encoding for streaming response
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "");

  server.sendContent("[");

  bool first = true;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();

    // Check if valid line
    // Format: Timestamp,UID,Name,Dept,Sem,Status,Contact(Optional)
    if (line.length() > 20 && line.startsWith(dateFilter)) {
      int c1 = line.indexOf(',');
      int c2 = line.indexOf(',', c1 + 1);
      int c3 = line.indexOf(',', c2 + 1);
      int c4 = line.indexOf(',', c3 + 1);
      int c5 = line.indexOf(',', c4 + 1);

      if (c5 > 0) {
        String ts = line.substring(11, 16); // HH:MM
        String name = line.substring(c2 + 1, c3);
        String dept = line.substring(c3 + 1, c4);
        String sem = line.substring(c4 + 1, c5);

        String status = "";
        String contact = "";

        int c6 = line.indexOf(',', c5 + 1);
        if (c6 > 0) {
          // New Format with Contact
          status = line.substring(c5 + 1, c6);
          contact = line.substring(c6 + 1);
        } else {
          // Old Format without Contact
          status = line.substring(c5 + 1);
          contact = "-";
        }
        status.trim();
        contact.trim();

        String chunk = "";
        if (!first)
          chunk += ",";
        chunk += "{\"time\":\"" + ts + "\",\"name\":\"" + name +
                 "\",\"dept\":\"" + dept + "\",\"sem\":\"" + sem +
                 "\",\"status\":\"" + status + "\",\"contact\":\"" + contact +
                 "\"}";

        server.sendContent(chunk);
        first = false;
      }
    }
  }
  server.sendContent("]");
  file.close();
}

void handleDownloadReport() {
  String dateFilter = server.arg("date");
  if (dateFilter.length() < 10) {
    server.send(400, "text/plain", "Date required");
    return;
  }

  File file = LittleFS.open("/attendance.csv", "r");
  if (!file) {
    server.send(404, "text/plain", "No logs");
    return;
  }

  server.sendHeader("Content-Type", "application/octet-stream");
  server.sendHeader("Content-Disposition",
                    "attachment; filename=\"report_" + dateFilter + ".csv\"");
  server.sendHeader("Connection", "close");

  server.sendContent("Time,Name,Department,Semester,Status,Contact\n");

  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.startsWith(dateFilter)) {
      int c1 = line.indexOf(',');
      int c2 = line.indexOf(',', c1 + 1);

      String timestamp = line.substring(11, 19);
      String rest = line.substring(c2 + 1); // Name...

      server.sendContent(timestamp + "," + rest + "\n");
    }
  }
  file.close();
}

void handleClearLogs() {
  String dateFilter = server.arg("date");
  if (dateFilter.length() < 10) {
    server.send(400, "text/plain", "Date filter required");
    return;
  }

  if (!LittleFS.exists("/attendance.csv")) {
    server.send(404, "text/plain", "No logs found");
    return;
  }

  File inFile = LittleFS.open("/attendance.csv", "r");
  File outFile = LittleFS.open("/temp.csv", "w");

  if (!inFile || !outFile) {
    server.send(500, "text/plain", "FS Error");
    return;
  }

  bool logsRemoved = false;

  while (inFile.available()) {
    String line = inFile.readStringUntil('\n');
    String trimmed = line;
    trimmed.trim();

    if (trimmed.length() > 0) {
      if (trimmed.startsWith(dateFilter)) {
        logsRemoved = true;
        // Skip this line (delete it)
      } else {
        outFile.println(trimmed); // Keep this line
      }
    }
  }

  inFile.close();
  outFile.close();

  LittleFS.remove("/attendance.csv");
  LittleFS.rename("/temp.csv", "/attendance.csv");

  // Recount if we deleted today's logs
  DateTime now = rtc.now();
  if (dateFilter == now.timestamp().substring(0, 10)) {
    countPresentToday();
  }

  if (logsRemoved)
    server.send(200, "text/plain", "Logs deleted for " + dateFilter);
  else
    server.send(200, "text/plain", "No logs found for " + dateFilter);
}

// --- Main Setup & Loop ---

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  noTone(BUZZER_PIN);

  Wire.begin();
  rtc.begin();
  SPI.begin();
  mfrc522.PCD_Init();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    Serial.println(F("SSD1306 Fail"));
  showBoot(); // Display Engine

  if (!LittleFS.begin(true))
    Serial.println("FS Fail");
  countPresentToday();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    showWiFiConnecting(retries); // Display Engine
    delay(500);
    retries++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.softAP("SmartAttendance", "12345678");
  }

  // Initial Idle Screen
  currentState = STATE_IDLE;

  server.on("/", handleRoot);
  server.on("/api/status", handleStatus);
  server.on("/api/students", handleGetStudents);
  server.on("/api/student", HTTP_POST, handleSaveStudent);
  server.on("/api/student", HTTP_DELETE, handleDeleteStudent);
  server.on("/api/routines", handleGetRoutines);
  server.on("/api/routine", HTTP_POST, handleSaveRoutine);
  server.on("/api/reset_routines", HTTP_POST, handleResetRoutines);
  server.on("/api/logs", handleGetLogs);
  server.on("/api/download_logs", handleDownloadReport);
  server.on("/api/clear_logs", HTTP_POST, handleClearLogs);
  server.begin();
}

void loop() {
  server.handleClient();

  // Update Display (Idle Clock or Message)
  String ip = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString()
                                              : "AP: 192.168.4.1";
  updateDisplay(rtc.now(), ip);

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();

    // --- Duplicate Check ---
    DateTime now = rtc.now();

    // Debug: Print what we are checking
    Serial.print("Scanned UID: ");
    Serial.println(uid);
    Serial.print("Is in Cache? ");
    Serial.println(scannedTodayUIDs.count(uid));

    if (scannedTodayUIDs.count(uid) > 0) {
      // ALREADY SCANNED TODAY
      Serial.println(">> Duplicate Blocked");
      lastScannedUID = uid;
      lastScannedTime = formatTime12h(now);
      lastScannedName = "Already Scanned";

      triggerScanError(uid);

      // Explicit Double Beep
      tone(BUZZER_PIN, 1000, 100);
      delay(150);
      tone(BUZZER_PIN, 1000, 100);
      delay(150);
      noTone(BUZZER_PIN);
      digitalWrite(BUZZER_PIN, LOW);

    } else {
      // NEW SCAN
      Serial.println(">> New Scan Processing");
      lastScannedUID = uid;
      lastScannedTime = formatTime12h(now);

      Student s = getStudent(uid);

      if (s.valid) {
        lastScannedName = s.name;
        String routineJson = getRoutineJSON(s.dept, s.sem);
        AttendanceStatus status =
            calculateStatus(rtc.now(), routineJson, false, false);

        triggerScanSuccess(s.name, getStatusString(status));

        logEvent(s, status); // Writes to CSV

        // Always mark as scanned if valid student, regardless of status details
        // ensuring we don't log them again today.
        scannedTodayUIDs.insert(uid);
        totalPresentToday = scannedTodayUIDs.size();
        Serial.println(">> Added to Cache. Total: " +
                       String(totalPresentToday));

        tone(BUZZER_PIN, 1000, 100);
        delay(150);
        noTone(BUZZER_PIN);
        digitalWrite(BUZZER_PIN, LOW);
      } else {
        lastScannedName = "";
        triggerScanError(uid);
        Serial.println(">> Unknown Card");

        tone(BUZZER_PIN, 200, 300);
        delay(300);
        noTone(BUZZER_PIN);
        digitalWrite(BUZZER_PIN, LOW);
      }
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}
