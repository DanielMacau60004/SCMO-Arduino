#include "cloud.h"
#include "internet.h"
#include "system.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }

  initFileSystem();

  //Wifi config setup
  //WifiConfig defaultWifiConfig = { "MEO-6B144F", "CA22156F4B" };
  //WifiConfig defaultWifiConfig = { "Rede do Lol", "miguel123" };
  //storeWifiConfig("{ \"ssid\": \"MEO-6B144F\", \"pwd\": \"CA22156F4B\" }");

  connectToWifi();

  if (isWifiConnected()) {
    fetchCurrentDate();

    startSystem();
  }
}

void loop() {
  loopSystem();
}