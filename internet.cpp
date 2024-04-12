#include "internet.h"

void connectToWifi() {

  //Initialize wifi config from file
  if (wifiConfig["ssid"].isNull() || wifiConfig["pwd"].isNull()) {
    readFile(CONFIG_FILE, wifiConfig);
    Serial.println("Config loaded");
  }

  const char* ssid = wifiConfig["ssid"];
  const char* password = wifiConfig["pwd"];
  WiFi.begin(ssid, password);

  for (int i = 0; WiFi.status() != WL_CONNECTED && i < WIFI_CONNECTION_TRIES; i++) {
    delay(WIFI_CONNECTION_DELAY);
    Serial.print(".");
  }

  if (isWifiConnected())
    Serial.println("\nWiFi connected");
}

void storeWifiConfig(char* json) {
  deserializeJson(wifiConfig, json);
  writeFile(CONFIG_FILE, wifiConfig);
}

bool isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}
