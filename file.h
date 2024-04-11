#ifndef FILE_H
#define FILE_H

#include <SPIFFS.h>
#include <ArduinoJson.h>

void initFileSystem(bool format = false);
void formatDisk();

void writeFile(const char* file, JsonDocument& json);
void readFile(const char* file, JsonDocument& json);

bool existFile(const char* file);
void deleteFile(const char* file);

#endif