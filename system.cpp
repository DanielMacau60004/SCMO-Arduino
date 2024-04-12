#include "system.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long lcdTime;

DHTesp dht;

void printLCD(String line1, String line2, int duration = 3) {
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  lcd.backlight();
  lcdTime = millis() + duration * 1000;
}

void startSystem() {
  Wire.begin(SDA, SCL);

  lcd.init();
  dht.setup(DHTPIN, DHTesp::DHT11);
  pinMode(MOTION, INPUT);

  ledcSetup(SERVO_CHN, SERVO_FRQ, SERVO_BIT);
  ledcAttachPin(SERVO_1, SERVO_CHN);
  ledcAttachPin(SERVO_2, SERVO_CHN);

  //TEMPORARY Start System Object
  deserializeJson(sys, "{\"active\": true,\"nextTime\": 0,\"currentTime\": 0,\"duration\": 10,\"hourToStart\": 8,\"rotation\": [true, false, true, true, false, true, false]}");
}

//Temporary
void test(DynamicJsonDocument json) {
  Serial.println("Received!");
}

void loopSystem() {
  if (onDuration <= millis()) {
    lcd.noBacklight();
    lcdTime = ULONG_MAX;
  }

  if()

  //Check if the system can run

}

void addData() {
  JsonArray jsonArray;
  if (sys.containsKey("data"))
    jsonArray = sys["data"].as<JsonArray>();
  else jsonArray = sys.createNestedArray("data");

  JsonObject root = jsonArray.createNestedObject();
  root["temp"] = dht.getTemperature();
  root["hum"] = dht.getHumidity();
  root["t"] = getCurrentDate();
}

void addStatus() {
    JsonArray jsonArray;
  if (sys.containsKey("status"))
    jsonArray = sys["status"].as<JsonArray>();
  else jsonArray = sys.createNestedArray("status");

  JsonObject root = jsonArray.createNestedObject();
  root["stats"] = RUNNING;
}
