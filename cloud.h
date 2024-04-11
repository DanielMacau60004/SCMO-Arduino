#ifndef CLOUD_H
#define CLOUD_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

typedef void (*fetchFunction)(DynamicJsonDocument);

void getRequest(const char* url, fetchFunction fun);
void postRequest(const char* url, JsonObject& obj, fetchFunction fun);
void fetchCurrentDate();
unsigned long getCurrentDate();

#endif