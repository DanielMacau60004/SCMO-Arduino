#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <ArduinoJson.h>

enum SystemState {
  RUNNING,  //WSystem is running
  PAUSED,  //Motion sensor is fired
  WAITING   //System is waiting for the next event/ next irrigation
};

extern DynamicJsonDocument wifiConfig;

extern DynamicJsonDocument sys;



#endif