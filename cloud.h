#ifndef CLOUD_H
#define CLOUD_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//#include <Arduino.h>

typedef void (*fetchFunction)(DynamicJsonDocument);

void initCloud();
void getRequest(const char* endpoint, fetchFunction fun);
void postRequest(const char* endpoint, JsonObject& obj, fetchFunction fun);

#endif