#include "system.h"

//LCD stuff
#define SDA 14  //Define SDA pins
#define SCL 13  //Define SCL pins
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long onDuration;

//DHT stuff
#define DHTPIN 19      // Define the pin to which the DHT sensor is connected
#define DHTTYPE DHT11  // Define the type of DHT sensor (DHT22 or DHT11)
DHTesp dht;

System sys;

void printLCD(String line1, String line2, int duration = 3) {
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  lcd.backlight();
  onDuration = millis() + duration * 1000;
}

void startSystem() {
  Wire.begin(SDA, SCL);

  lcd.init();
  dht.setup(DHTPIN, DHTesp::DHT11);
}

void test(DynamicJsonDocument json) {
  Serial.println("Received!");
}

void loopSystem() {
  if (onDuration <= millis()) {
    lcd.noBacklight();
    onDuration = ULONG_MAX;
  }

  SystemDataNot data = getData();
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["temperature"] = data.temperature;
  root["humidity"] = data.humidity;
  root["time"] = data.time;

  postRequest("http://85.244.117.107/test", root, test);
  delay(2000);

}

SystemStatusNot getStatus() {
  SystemStatusNot notification = { sys.status, getCurrentDate() };
  return notification;
}

SystemDataNot getData() {
  SystemDataNot notification = { dht.getTemperature(), dht.getHumidity(), getCurrentDate() };
  return notification;
}
