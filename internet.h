#ifndef INTERNET_H
#define INTERNET_H

#include <BluetoothSerial.h>
#include <WiFi.h>
#include "system.h"
#include "structures.h"
#include "file.h"

#define CONFIG_FILE "/config.json"

#define WIFI_CONNECTION_DELAY 500
#define WIFI_CONNECTION_TRIES 100

void initWifi();
void connectToWifi();
void storeWifiConfig(char* json);
bool isWifiConnected();

#endif