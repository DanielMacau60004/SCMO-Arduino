#include "bluetooth.h"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


BLEService bleService(SERVICE_UUID);
BLECharacteristic bleCharacteristic(CHARACTERISTIC_UUID, BLERead | BLEWrite | BLENotify, 128);

bool deviceConnected = false;
unsigned long lastNotifyTime = 0;
std::string receivedData;

std::vector<std::string> splitString(const std::string &str, char delimiter) {
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;

  while (getline(ss, token, delimiter)) {
    tokens.push_back(token);
  }

  return tokens;
}

void auth(std::string name, std::string pwd) {
  bool result = name == SYSTEM_ID && pwd == SYSTEM_PWD;
  std::string resultString = result ? "1" : "-1";
  bleCharacteristic.writeValue(resultString.c_str());
}

void testWIFI(std::string name, std::string pwd, std::string json) {
  if (!(name == SYSTEM_ID && pwd == SYSTEM_PWD)) {
    bleCharacteristic.writeValue("-1");  //No permission
    return;
  }

  storeWifiConfig(json.c_str());
  connectToWifi(20);

  bool connected = isWifiConnected();
  std::string resultString = connected ? "1" : "0";

  bleCharacteristic.writeValue(resultString.c_str());
}

void connectToWIFI(std::string name, std::string pwd, std::string json) {
  if (!(name == SYSTEM_ID && pwd == SYSTEM_PWD)) {
    bleCharacteristic.writeValue("-1");  //No permission
    return;
  }

  storeWifiConfig(json.c_str());
  connectToWifi();
}

void characteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  std::string value(reinterpret_cast<const char *>(characteristic.value()), characteristic.valueLength());

  receivedData += value;
  size_t newlinePos;

  while ((newlinePos = receivedData.find('*')) != std::string::npos && newlinePos == receivedData.length() - 1) {
    std::string message = receivedData.substr(0, newlinePos);

    std::vector<std::string> tokens = splitString(message, ' ');

    if (tokens[0] == "auth" && tokens.size() == 3)
      auth(tokens[1], tokens[2]);
    if (tokens[0] == "twifi" && tokens.size() == 4)
      testWIFI(tokens[1], tokens[2], tokens[3]);
    else if (tokens[0] == "wifi" && tokens.size() == 4)
      connectToWIFI(tokens[1], tokens[2], tokens[3]);

    receivedData.erase(0, newlinePos + 1);
  }
}

void communicate(void *parameter) {
  while (true) {
    BLEDevice central = BLE.central();

    if (central) {
      deviceConnected = true;
      while (central.connected()) {}
      deviceConnected = false;
    }
  }
}

void initBluetooth() {
  BLE.begin();

  BLE.setLocalName(SYSTEM_ID);
  BLE.setAdvertisedService(bleService);

  bleService.addCharacteristic(bleCharacteristic);
  BLE.addService(bleService);

  bleCharacteristic.setEventHandler(BLEWritten, characteristicWritten);

  BLE.advertise();
  Serial.println("BLE device is now advertising");

  xTaskCreate(communicate, "Communicate", 10000, NULL, 1, NULL);
}

void connectBluetooth() {
  BLE.advertise();
  Serial.println("Bluetooth connected");
}

void disconnectBluetooth() {
  BLE.stopAdvertise();
  BLE.end();
  Serial.println("Bluetooth disconnected");
}
