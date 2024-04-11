#include "system.h"

System system;

void start() {

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
}

System getSystem() {
  return system;
}

SystemStatusNot getStatus() {
  SystemStatusNot not = { system.status, millis() };
  return not ;
}

SystemDataNot getData() {
  SystemDataNot not = { 0, 0, millis() };  // Add temperature & humidity
  return not ;
}