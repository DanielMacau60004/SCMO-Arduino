#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include "structures.h"
#include <limits.h>
#include <DHTesp.h>
#include "cloud.h"

#include <LiquidCrystal_I2C.h>
#include <Wire.h>


void startSystem();
void loopSystem();
SystemStatusNot getStatus();
SystemDataNot getData();
#endif