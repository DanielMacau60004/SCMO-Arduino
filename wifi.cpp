#include "wifi.h"

const char* CONFIG_FILE = "/config.json";

const int WIFI_CONNECTION_DELAY = 500;
const int WIFI_CONNECTION_TRIES = 10;

const char* KEY_SSID = "ssid";
const char* KEY_PWD = "pwd";

WifiConfig wifiConfig;

void connectToWifi() {

  DynamicJsonDocument jsonDocument(256);
  //Initialize wifi config from file
  if (wifiConfig.WIFI_SSID == NULL || wifiConfig.WIFI_PWD == NULL) {
  
    readFile(CONFIG_FILE, jsonDocument);

    wifiConfig.WIFI_SSID = strdup(jsonDocument[KEY_SSID]);
    wifiConfig.WIFI_PWD = strdup(jsonDocument[KEY_PWD]);

    Serial.println("Config loaded");
  }

  WiFi.begin(wifiConfig.WIFI_SSID, wifiConfig.WIFI_PWD);

  for (int i = 0; WiFi.status() != WL_CONNECTED && i < WIFI_CONNECTION_TRIES; i++) {
    delay(WIFI_CONNECTION_DELAY);
    Serial.print(".");
  }

  if (isWifiConnected())
    Serial.println("\nWiFi connected");
}

void storeWifiConfig(WifiConfig& config) {
  DynamicJsonDocument jsonDocument(256);

  jsonDocument[KEY_SSID] = config.WIFI_SSID;
  jsonDocument[KEY_PWD] = config.WIFI_PWD;
  writeFile(CONFIG_FILE, jsonDocument);

  wifiConfig = config;
}

bool isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}