#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ArduinoBLE.h>
#include <sstream>
#include <vector>
#include "internet.h"
#include "system.h"

enum ResponseCode {
  SUCCESS,
  DENIED,
  FAILED
};

void initBluetooth();
void connectBluetooth();
void disconnectBluetooth();

#endif