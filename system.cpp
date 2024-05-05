#include "system.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long lcdTime;

DHTesp dht;

//System variables
SystemState currentState = WAITING;
unsigned long lastDate;
unsigned long timeRunnig;
unsigned long timePausing;


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

  //TODO Fetch data from cloud, and from file
  deserializeJson(sys, "{\"id\": \"arduino\",\"active\": true,\"duration\": 10,\"hourToStart\": 8,\"rotation\": [true, false, true, true, false, true, false]}");
}

void receiveMsg(DynamicJsonDocument json) {
  sys = json;  //Update data from cloud
  Serial.println("Message received");
}

//TODO TEMP
#define TIME_TO_POST 10
int a;
//TODO TEMP

void loopSystem() {
  if (lcdTime <= millis()) {
    lcd.noBacklight();
    lcdTime = ULONG_MAX;
  }

  //If the system was stopped
  if (!sys["active"].as<bool>) {
    break;
  }

  switch (currentState) {

    case RUNNING:

      //Stop state
      if (timeRunnig >= sys["duration"].as<unsigned long>() * 60) {
        currentState = WAITING;
        addStatus();
        break;
      }

      unsigned long currentDate = getCurrentDate();
      timeRunnig = currentDate - lastDate;
      lastDate = currentDate;

      //Run things...

      //Check if the movement sensor fired
      if(digitalRead(MOTION)) {
        currentState = PAUSED;
        timePausing = 0;
        addStatus();
        break;
      }

      //Move sensors
      ledcWrite(SERVO_CHN, 1)

      //Fetch data constantly from the cloud
      pupdateCloud();

      break;

    case WAITING:
      unsigned long currentDate = getCurrentDate();
      unsigned long getcurrenthour = ....;  //TODO complete this
      unsigned long hourToStart = sys["hourToStart"].as<unsigned long>();
      unsigned long dayOfTheWeek = ....;  //TODO complete this

      bool needsToAct = sys["hourToStart"].as<JsonArray>()[dayOfTheWeek].as<bool>;

      //Day off :D
      if (!needsToAct) break;

      // Starting state
      if (getcurrenthour == hourToStart) {  //Check if shoud act!
        currentState = RUNNING;
        lastDate = currentDate;
        timeRunnig = 0;
        addStatus();
        break;
      }

      break;

    case PAUSED:
      unsigned long currentDate = getCurrentDate();
      timePausing = currentDate - lastDate;

      //Do nothing
      if(timePausing >= TIME_PAUSING) {
        currentState = RUNNING;
        lastDate = currentDate;
        addStatus();
      }

      break;

      //TODO do this on fixed time intervals
      writeFile(SYSTEM_FILE);
      updateCloud();
      addData();
  }



  //*******************************************************************
  //Working, don't delete this!
  /*if (a >= TIME_TO_POST) {
    addData();
    addStatus();
    putRequest("https://scmu.azurewebsites.net/rest/boards/{id}/arduino", sys, receiveMsg);
    a = 0;
    printLCD("Data sent!","",2);
  }
  a++;*/
  //*******************************************************************

  delay(1000);
}

void updateCloud() {
  putRequest("https://scmu.azurewebsites.net/rest/boards/"+sys["id"].as<String>+"/arduino", sys, receiveMsg);
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
  root["status"] = currentState;
  root["t"] = getCurrentDate();
}
