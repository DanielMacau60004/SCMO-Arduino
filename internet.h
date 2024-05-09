#ifndef INTERNET_H
#define INTERNET_H

#include <WiFi.h>
#include "structures.h"
#include "file.h"

#define CONFIG_FILE "/config.json"

#define WIFI_CONNECTION_DELAY 500
#define WIFI_CONNECTION_TRIES 100

void connectToWifi();
void storeWifiConfig(char* json);
bool isWifiConnected();

#endif