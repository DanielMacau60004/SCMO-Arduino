#include "file.h"

File dataFile;

void initFileSystem(bool format) {

  if (!SPIFFS.begin(format)) {
    Serial.println("File system initialization failed!");
    return;
  }

  Serial.println("File system initialized!");
}

void formatDisk() {
  if (SPIFFS.format()) {
    Serial.println("File system formatted successfully");
  } else {
    Serial.println("Error formatting file system");
  }
}

void writeFile(const char* file, JsonDocument& json, char * mode) {
  dataFile = SPIFFS.open(file, mode);
  if (!dataFile) {
    Serial.println("Error opening file for writing!");
    return;
  }

  String jsonString;
  if (serializeJson(json, jsonString) == 0) {
    Serial.println("Failed to append JSON to file");
    return;
  }

  dataFile.println(jsonString);
  dataFile.close();
}

void writeFile(const char* file, JsonDocument& json) {
  writeFile(file, json, "w");
}

void appendFile(const char* file, JsonDocument& json) {
  writeFile(file, json, "a");
}

void readFile(const char* file, JsonDocument& json) {
  dataFile = SPIFFS.open(file, "r");
  if (!dataFile) {
    Serial.println("Error opening file for writing!");
    return;
  }

  DeserializationError error = deserializeJson(json, dataFile);
  if (error) {
    Serial.println("Failed to parse JSON from file");
    return;
  }

  dataFile.close();
}

void readAppendFile(const char* file, JsonArray& jsonArray) {
  File dataFile = SPIFFS.open(file, "r");
  if (!dataFile) {
    Serial.println("Error opening file for reading!");
    return;
  }

  String line;
  while (dataFile.available()) {
    line = dataFile.readStringUntil('\n');
    
    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, line);
    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
    } else {
      jsonArray.add(doc.as<JsonObject>());
    }
  }

  dataFile.close();
}


bool existFile(const char* file) {
  return SPIFFS.exists(file);
}

void deleteFile(const char* file) {
  if (!SPIFFS.exists(file)) {
    Serial.println("File does not exist!");
    return;
  }

  if (!SPIFFS.remove(file)) {
    Serial.println("Error deleting file!");
    return;
  }

  Serial.println("File deleted successfully.");
}
