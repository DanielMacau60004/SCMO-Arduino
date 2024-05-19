#ifndef INTERNET_H
#define INTERNET_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include "system.h"
#include "structures.h"
#include "file.h"
#include "bluetooth.h"

#define CONFIG_FILE "/config.json"

#define WIFI_CONNECTION_DELAY 500
#define WIFI_CONNECTION_TRIES 100

void initWifi();
void connectToWifi(int tries);
void connectToWifi();
void storeWifiConfig(const char* json);
bool isWifiConnected();

#endif