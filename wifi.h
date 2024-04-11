#pragma once

#ifndef WIFI_H
#define WIFI_H

#include <WiFi.h>
#include "structures.h"
#include "file.h"

void connectToWifi();
void storeWifiConfig(WifiConfig& config);
bool isWifiConnected();

#endif