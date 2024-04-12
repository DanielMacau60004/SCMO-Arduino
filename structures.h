#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <ArduinoJson.h>

enum SystemState {
  RUNNING,  //WSystem is running
  PAUSING,  //Motion sensor is fired
  WAITING   //System is waiting for the next event/ next irrigation
};

extern DynamicJsonDocument wifiConfig;

extern DynamicJsonDocument sys;



struct SystemStatus {
  SystemState status;

  //Computate abg temperature and humidity
  float sumTemperature = 0;
  float sumHumididty = 0;
  unsigned int totalRecords = 0;

  unsigned long start;
  unsigned long end;  //Time when the event was registered
  SystemStatus* next;
};



#endif