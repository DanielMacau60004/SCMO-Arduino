#include "internet.h"

void reconnect(void *parameter) {
  while(true) {

    if(!isWifiConnected()) {
      readFile(CONFIG_FILE, wifiConfig);
      connectToWifi();
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

bool isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void initWifi() {
  connectToWifi();
  while (!isWifiConnected()) {
    connectToWifi();
  }

  xTaskCreate(reconnect,"Reconnect",10000,NULL,2,NULL);
}

void connectToWifi(int tries) {
  readFile(CONFIG_FILE, wifiConfig);

  const char* ssid = wifiConfig["ssid"];
  const char* pwd = wifiConfig["pwd"];
  WiFi.begin(ssid, pwd);

  for (int i = 0; WiFi.status() != WL_CONNECTED && i < tries; i++) {
    delay(WIFI_CONNECTION_DELAY);
    Serial.print(".");
  }

  if (isWifiConnected())
    Serial.println("\nWiFi connected");
}

void connectToWifi() {
  connectToWifi(WIFI_CONNECTION_TRIES);
}

void storeWifiConfig(const char* json) {
  deserializeJson(wifiConfig, json);
  writeFile(CONFIG_FILE, wifiConfig);
}
