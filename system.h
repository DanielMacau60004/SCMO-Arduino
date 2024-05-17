#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include "structures.h"
#include <limits.h>
#include <DHTesp.h>
#include "cloud.h"
#include <time.h>
#include "file.h"

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//Global stuff
#define SYSTEM_ID "arduino01"
#define BASE_URL "https://scmu.azurewebsites.net/rest/boards/"
#define PUT_ENDPOINT "/arduino"
#define RESET_ENDPOINT "/reset"
#define SYSTEM_FILE "/system.json"
#define TIME_PAUSING 5 * 60 //in seconds
#define TIME_FETCHING 3 //in seconds

#define SPEED_UP_WAITING 60 //in seconds
#define SPEED_UP_ACTING 1 //in seconds

#define HUMIDITY_THRESHOLD 60
#define TEMPERATURE_THRESHOLD 15

//LCD stuff
#define SDA 14
#define SCL 13
//use printLCD

//DHT stuff
#define DHTPIN 19
#define DHTTYPE DHT11
//use dht dht.getTemperature(), dht.getHumidity()

//Motion stuff
#define MOTION 15
//use motion digitalRead(MOTION)

//Servo stuff
#define SERVO_1 10
#define SERVO_2 18
#define SERVO_CHN 0 
#define SERVO_FRQ 10
#define SERVO_BIT 12
//use servos ledcWrite(SERVO_CHN, 1);

void startSystem();
void loopSystem();
void addData();
void addStatus(SystemState state);

#endif