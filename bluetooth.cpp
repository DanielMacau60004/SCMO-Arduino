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
  bleCharacteristic.writeValue(String(result ? SUCCESS : DENIED).c_str());
}

void connectToWIFI(std::string name, std::string pwd, std::string json) {
  if (!(name == SYSTEM_ID && pwd == SYSTEM_PWD)) {
    bleCharacteristic.writeValue(String(DENIED).c_str());  //No permission
    return;
  }

  storeWifiConfig(json.c_str());
  connectToWifi(10);

  bool connected = isWifiConnected();
  bleCharacteristic.writeValue(String(connected ? SUCCESS : FAILED).c_str());
}

void characteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  std::string value(reinterpret_cast<const char *>(characteristic.value()), characteristic.valueLength());

  receivedData += value;
  size_t newlinePos;

  Serial.println(receivedData.c_str());
  while ((newlinePos = receivedData.find('\n')) != std::string::npos && newlinePos == receivedData.length() - 1) {
    std::string message = receivedData.substr(0, newlinePos);

    std::vector<std::string> tokens = splitString(message, '\t');

    if (tokens[0] == "auth" && tokens.size() == 3)
      auth(tokens[1], tokens[2]);
    else if (tokens[0] == "wifi" && tokens.size() == 4)
      connectToWIFI(tokens[1], tokens[2], tokens[3]);
    else
      bleCharacteristic.writeValue(String(FAILED).c_str());
    receivedData.erase(0, newlinePos + 1);
  }
}

void communicate(void *parameter) {
  while (true) {
    BLEDevice central = BLE.central();

    if (central) {
      Serial.println("BLEDevice connected!");
      deviceConnected = true;
      while (central.connected()) {}
      deviceConnected = false;
      Serial.println("BLEDevice disconnected!");
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
