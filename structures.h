#pragma once

#ifndef STRUCTURES_H
#define STRUCTURES_H

struct WifiConfig {
  const char* WIFI_SSID;  //Wifi name
  const char* WIFI_PWD;   //Wifi password
};

enum SystemState {
  RUNNING,  //WSystem is running
  PAUSING,  //Motion sensor is fired
  WAITING   //System is waiting for the next event/ next irrigation
};

struct System {
  bool active;         // If it is running or not
  SystemState status;  //Current status

  unsigned long nextTime;
  unsigned ong currentTime;  //Time that start the

  unsigned int duration;      //time in minutes
  unsigned int hourToStart;   //hour to start the system
  bool rotation[7];  //week days
};

struct SystemStatusNot {
  SystemState status;
  unsigned long time;  //Time when the event was registered
};

struct SystemDataNot {
  float temperature;
  float humidity;
  unsigned long time;  //Time when the event was registered
};

#endif