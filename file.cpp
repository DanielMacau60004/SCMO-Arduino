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
