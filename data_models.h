#ifndef DATA_MODELS_H
#define DATA_MODELS_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
// --- Data Structures ---
struct Student {
  String uid;
  String name;
  String dept;
  String sem;
  String roll;
  String reg;
  String contact; // Mobile Number
  String blood;   // Blood Group
  bool valid;     // To check if found
};
// --- File I/O Helpers ---
// Get all students for the management list
String getAllStudentsJSON() {
  File file = LittleFS.open("/students.json", "r");
  if (!file)
    return "[]";
  DynamicJsonDocument doc(16384);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error)
    return "[]";
  DynamicJsonDocument arrayDoc(16384);
  JsonArray array = arrayDoc.to<JsonArray>();
  JsonObject root = doc.as<JsonObject>();
  for (JsonPair kv : root) {
    JsonObject s = array.createNestedObject();
    s["uid"] = kv.key().c_str();
    JsonObject val = kv.value().as<JsonObject>();
    s["name"] = val["name"];
    s["dept"] = val["dept"];
    s["sem"] = val["sem"];
    s["roll"] = val["roll"];
    s["reg"] = val["reg"];
    s["contact"] = val["contact"];
    s["blood"] = val["blood"];
  }
  String output;
  serializeJson(array, output);
  return output;
}
// Load a specific student by UID
Student getStudent(String uid) {
  Student s;
  s.valid = false;
  File file = LittleFS.open("/students.json", "r");
  if (!file)
    return s;
  DynamicJsonDocument doc(16384);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error)
    return s;
  if (doc.containsKey(uid)) {
    JsonObject obj = doc[uid];
    s.uid = uid;
    s.name = obj["name"].as<String>();
    s.dept = obj["dept"].as<String>();
    s.sem = obj["sem"].as<String>();
    s.roll = obj["roll"].as<String>();
    s.reg = obj["reg"].as<String>();
    s.contact = obj["contact"].as<String>();
    s.blood = obj["blood"].as<String>();
    s.valid = true;
  }
  return s;
}
// Save or Update a student
void saveStudent(Student s) {
  DynamicJsonDocument doc(16384);
  File file = LittleFS.open("/students.json", "r");
  if (file) {
    deserializeJson(doc, file);
    file.close();
  }
  JsonObject obj = doc.createNestedObject(s.uid);
  obj["name"] = s.name;
  obj["dept"] = s.dept;
  obj["sem"] = s.sem;
  obj["roll"] = s.roll;
  obj["reg"] = s.reg;
  obj["contact"] = s.contact;
  obj["blood"] = s.blood;
  file = LittleFS.open("/students.json", "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
  }
}
// Delete a student
void deleteStudent(String uid) {
  DynamicJsonDocument doc(16384);
  File file = LittleFS.open("/students.json", "r");
  if (file) {
    deserializeJson(doc, file);
    file.close();
  }
  doc.remove(uid);
  file = LittleFS.open("/students.json", "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
  }
}
// Get Routine JSON for a Dept & Sem
String getRoutineJSON(String dept, String sem) {
  File file = LittleFS.open("/routines.json", "r");
  if (!file)
    return "{}";
  DynamicJsonDocument doc(8192);
  deserializeJson(doc, file);
  file.close();
  if (doc.containsKey(dept) && doc[dept].containsKey(sem)) {
    String output;
    serializeJson(doc[dept][sem], output);
    return output;
  }
  return "{}";
}
// Save Routine (Accepts raw JSON string from Web UI)
void saveRoutineJSON(String dept, String sem, String jsonContent) {
  DynamicJsonDocument doc(8192);
  File file = LittleFS.open("/routines.json", "r");
  if (file) {
    deserializeJson(doc, file);
    file.close();
  }
  if (!doc.containsKey(dept))
    doc.createNestedObject(dept);
  // Parse the new routine data
  DynamicJsonDocument newRoutine(1024);
  deserializeJson(newRoutine, jsonContent);
  doc[dept][sem] = newRoutine.as<JsonObject>();
  file = LittleFS.open("/routines.json", "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
  }
}
// Get All Routines (for UI tree view)
String getAllRoutinesJSON() {
  File file = LittleFS.open("/routines.json", "r");
  if (!file) {
    // Attempt to create it if missing
    File create = LittleFS.open("/routines.json", "w");
    if (create) {
      create.print("{}");
      create.close();
    }
    return "{}";
  }
  String output = file.readString();
  file.close();
  output.trim();
  if (output.length() == 0)
    return "{}";
  // Validate AND Clean JSON
  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, output);
  if (error) {
    Serial.print("Corrupt routine file: ");
    Serial.println(error.c_str());
    // Auto-repair
    File repair = LittleFS.open("/routines.json", "w");
    if (repair) {
      repair.print("{}");
      repair.close();
    }
    return "{}";
  }
  // Re-serialize to ensure 100% valid JSON for browser
  String cleanOutput;
  serializeJson(doc, cleanOutput);
  return cleanOutput;
}
#endif