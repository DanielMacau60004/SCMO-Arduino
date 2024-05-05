#ifndef CLOUD_H
#define CLOUD_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define URL "http://85.244.117.107"
#define URL_DATETIME_UTC "https://worldtimeapi.org/api/timezone/utc"

typedef void (*fetchFunction)(DynamicJsonDocument);

void getRequest(const char* url, fetchFunction fun);
void postRequest(const char* url, DynamicJsonDocument& obj, fetchFunction fun);
void putRequest(const char* url, DynamicJsonDocument& obj, fetchFunction fun);
void fetchCurrentDate();
unsigned long getCurrentDate();

#endif