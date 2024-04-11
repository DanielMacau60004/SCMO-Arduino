#include "cloud.h"
#include "wifi.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }

  initFileSystem();

  //Wifi config setup
  //WifiConfig defaultWifiConfig = { "MEO-6B144F", "CA22156F4B" };
  //storeWifiConfig(defaultWifiConfig);

  connectToWifi();
  initCloud();
}

void loop() {
}