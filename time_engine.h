#ifndef TIME_ENGINE_H
#define TIME_ENGINE_H
#include "data_models.h"
#include <Arduino.h>
#include <RTClib.h>
// Convert "HH:MM" string to minutes from midnight
int timeToMinutes(String timeStr) {
  if (timeStr.length() < 5)
    return -1;
  int hr = timeStr.substring(0, 2).toInt();
  int min = timeStr.substring(3, 5).toInt();
  return (hr * 60) + min;
}
// Convert minutes to "HH:MM AM/PM" for display
String minutesToDisplay(int totalMins) {
  int hr = totalMins / 60;
  int min = totalMins % 60;
  String ampm = "AM";
  if (hr >= 12) {
    ampm = "PM";
    if (hr > 12)
      hr -= 12;
  }
  if (hr == 0)
    hr = 12;
  String s = String(hr) + ":";
  if (min < 10)
    s += "0";
  s += String(min) + " " + ampm;
  return s;
}
// Helper to format DateTime to 12h string "08:30 PM"
String formatTime12h(DateTime dt) {
  int hr = dt.hour();
  int min = dt.minute();
  return minutesToDisplay((hr * 60) + min);
}
enum AttendanceStatus {
  STATUS_PRESENT,
  STATUS_LATE,
  STATUS_EXIT,
  STATUS_GATE_PASS,
  STATUS_EMERGENCY,
  STATUS_DENIED,
  STATUS_ALREADY_IN,
  STATUS_ERROR
};
String getStatusString(AttendanceStatus s) {
  switch (s) {
  case STATUS_PRESENT:
    return "Present";
  case STATUS_LATE:
    return "Late";
  case STATUS_EXIT:
    return "Exit";
  case STATUS_GATE_PASS:
    return "Gate Pass";
  case STATUS_EMERGENCY:
    return "Emergency";
  case STATUS_DENIED:
    return "Denied";
  case STATUS_ALREADY_IN:
    return "Already In";
  default:
    return "Error";
  }
}
AttendanceStatus calculateStatus(DateTime now, String routineJson,
                                 bool isEmergency, bool alreadyIn) {
  if (routineJson == "{}" || routineJson == "")
    return STATUS_ERROR;
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, routineJson);
  JsonObject r = doc.as<JsonObject>();
  int currentMins = (now.hour() * 60) + now.minute();
  int startMins = timeToMinutes(r["start"].as<String>());
  int endMins = timeToMinutes(r["end"].as<String>());
  // Emergency Override
  if (isEmergency)
    return STATUS_EMERGENCY;
  // Exit Logic (End of day)
  if (currentMins >= endMins) {
    return STATUS_EXIT;
  }
  // Gate Pass Logic (Check all breaks)
  bool isBreak = false;
  JsonArray breaks = r["breaks"].as<JsonArray>();
  for (JsonObject b : breaks) {
    int bs = timeToMinutes(b["s"].as<String>());
    int be = timeToMinutes(b["e"].as<String>());
    if (currentMins >= bs && currentMins <= be) {
      isBreak = true;
      break;
    }
  }
  if (alreadyIn) {
    if (isBreak)
      return STATUS_GATE_PASS;
    if (currentMins >= endMins)
      return STATUS_EXIT;
    return STATUS_DENIED; // Trying to exit during class without emergency
  }
  // Entry Logic
  if (currentMins < startMins + 15) { // 15 min grace period
    return STATUS_PRESENT;
  } else if (currentMins < endMins) {
    return STATUS_LATE;
  }
  return STATUS_DENIED; // Should be covered by Exit logic, but fallback
}
#endif