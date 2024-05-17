#include "internet.h"

/*BluetoothSerial SerialBT;

void fetchFromBluetooth() {
  SerialBT.begin(SYSTEM_ID);
  Serial.println("Bluetooth initialized");

  // Wait for Bluetooth connection
  while (!SerialBT.connected()) {
    delay(100);
  }
  Serial.println("Bluetooth connected");

  const int bufferSize = 256;
  char receivedData[bufferSize];
  int dataIndex = 0;

  while (SerialBT.available()) {
    char incomingByte = SerialBT.read();
    if (incomingByte == '\n') {
      receivedData[dataIndex] = '\0';
      SerialBT.end();
      break;
    }
    if (dataIndex < bufferSize - 1) {
      receivedData[dataIndex++] = incomingByte;
    }
    delay(2);
  }

  Serial.println("Received JSON: ");
  Serial.println(receivedData);
  //storeWifiConfig(receivedData)
}*/

bool isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void initWifi() {

  //Initialize wifi config from file
  readFile(CONFIG_FILE, wifiConfig);
  Serial.println("Config loaded");

  //if (wifiConfig["ssid"].isNull() || wifiConfig["pwd"].isNull())  //Don't have nothing stored
  //  fetchFromBluetooth();

  connectToWifi();
  while (!isWifiConnected()) {
    //fetchFromBluetooth();
    connectToWifi();
  }
}

void connectToWifi() {
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
