#include "esp32-hal.h"
#include "system.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long lcdTime;

DHTesp dht;

//System variables
SystemState currentState;
unsigned int state;
unsigned long lastDate;
unsigned long timeRunning;
unsigned long timePaused;

unsigned long lastFetch;

unsigned long newDate;
unsigned long newMilli;

unsigned long lastMotion;
bool hasMotion = false;

unsigned int lastAction;

size_t originalStatusSize;

void printLCD(String line1, String line2, int duration = 3) {
  lcd.setCursor(0, 0);
  lcd.print(line1 + "           ");
  lcd.setCursor(0, 1);
  lcd.print(line2 + "           ");
  lcd.backlight();
  lcdTime = millis() + duration * 1000;
}

String formatTime(int timeValue) {
  char formattedTime[3];
  sprintf(formattedTime, "%02d", timeValue);
  return String(formattedTime);
}

void cloudMessage(DynamicJsonDocument json) {
  JsonArray originalStatusArray = sys["status"].as<JsonArray>();

  int size = originalStatusArray.size() - originalStatusSize;
  int a_states[size];
  unsigned long a_times[size];

  for (size_t i = originalStatusSize; i < size; ++i) {
    JsonObject newElement = originalStatusArray[i];
    a_states[i] = newElement["state"].as<int>();
    a_times[i] = newElement["t"].as<unsigned long>();
  }

  sys = json;

  for (size_t i = originalStatusSize; i < size; ++i) {
    JsonObject root = sys["status"].createNestedObject();
    root["status"] = a_states[i];
    root["t"] = a_times[i];
  }
}

void cloudEmpty(DynamicJsonDocument json) {}

bool hasMotions() {
  unsigned long currentMillis = millis();
  if (lastMotion > currentMillis)
    return hasMotion;

  digitalWrite(SONIC_TRIGGER, LOW);
  delayMicroseconds(2);

  digitalWrite(SONIC_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(SONIC_TRIGGER, LOW);

  long duration = pulseIn(SONIC_ECHO, HIGH);
  float distance = (duration * .0343) / 2;

  lastMotion = currentMillis + SONIC_DELAY * 1000;
  hasMotion = distance < SONIC_DISTANCE_TRIGGER;
  return hasMotion;
}

void periodicFecth(void *parameter) {
  while (true) {
    sys["lastUpdate"] = getCurrentDate();
    sys["currentDate"] = newDate;
    sys["currentState"] = currentState;
    addData();

    originalStatusSize = sys["status"].as<JsonArray>().size();
    putRequest(BASE_URL SYSTEM_ID PUT_ENDPOINT, sys, cloudMessage);
    state = sys["state"].as<unsigned int>();
    sys["state"] = -1;

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void startSystem() {
  Wire.begin(SDA, SCL);

  lcd.init();
  dht.setup(DHTPIN, DHTesp::DHT11);
  pinMode(SONIC_TRIGGER, OUTPUT);
  pinMode(SONIC_ECHO, INPUT);

  ledcSetup(SERVO_CHN, SERVO_FRQ, SERVO_BIT);
  ledcAttachPin(SERVO_1, SERVO_CHN);
  ledcAttachPin(SERVO_2, SERVO_CHN);

  //Fetch data from the cloud and file
  Serial.println("Starting System!");
  while (sys == NULL || sys.size() == 0) {
    getRequest(BASE_URL SYSTEM_ID, cloudMessage);
    delay(100);
    Serial.print(".");
  }

  Serial.println("\nDeleting old data!");
  getRequest(BASE_URL SYSTEM_ID RESET_ENDPOINT, cloudEmpty);
  currentState = WAITING;
  Serial.println("\nSystem Started!");

  state = sys["state"].as<unsigned int>();

  newDate = getCurrentDate();
  newMilli = millis();

  xTaskCreate(periodicFecth, "PeriodicFetch", 10000, NULL, 1, NULL);
}

void running(unsigned int state) {
  //Remote stop
  if (state == WAITING) {
    currentState = WAITING;
    addStatus(CANCELED_USER);
    ledcWrite(SERVO_CHN, 0);
    return;
  }

  //Normal Stop
  if (state == WAITING || timeRunning >= sys["duration"].as<unsigned long>() * 60) {
    currentState = WAITING;
    addStatus(currentState);
    ledcWrite(SERVO_CHN, 0);
    return;
  }

  timeRunning += newDate - lastDate;
  lastDate = newDate;

  unsigned long minutes = (timeRunning % 3600) / 60;
  printLCD("Running!", String(minutes) + " mins", 2);

  //Run things...
  ledcWrite(SERVO_CHN, 1);

  //Check if the movement sensor fired
  if (state == PAUSED || hasMotions()) {
    ledcWrite(SERVO_CHN, 0);
    currentState = PAUSED;
    timePaused = 0;
    addStatus(currentState);
    return;
  }
}

void waiting(unsigned int state) {
  time_t timeInSeconds = newDate;
  struct tm *local_time = localtime(&timeInSeconds);
  unsigned long hourToStart = sys["hourToStart"].as<unsigned long>();
  unsigned long hour = hourToStart / 60;
  unsigned long minutes = hourToStart % 60;

  bool needsToAct = sys["rotation"].as<JsonArray>()[local_time->tm_wday];
  char buffer[80];

  strftime(buffer, sizeof(buffer), "Waiting! %a", local_time);
  printLCD(buffer, String(formatTime(local_time->tm_hour)) + ":" + String(formatTime(local_time->tm_min)) + " T:" + String((int)dht.getTemperature()) + " H:" + String((int)dht.getHumidity()) + "%", 2);

  if (!needsToAct)
    return;

  // Starting state
  if (local_time->tm_hour == hour && local_time->tm_yday > lastAction) {
    if (dht.getHumidity() > HUMIDITY_THRESHOLD || dht.getTemperature() < TEMPERATURE_THRESHOLD) {  //Check weather conditions
      addStatus(CANCELED_SYSTEM);
      return;
    }
    currentState = RUNNING;

    lastAction = local_time->tm_yday;
    newDate = (newDate / (24 * 60 * 60)) * (24 * 60 * 60) + hourToStart * 60;
    lastDate = newDate;
    timeRunning = 0;
    addStatus(currentState);
    return;
  }
}

void paused(unsigned int state) {

  timePaused += newDate - lastDate;
  lastDate = newDate;

  unsigned long minutes = (timePaused % 3600) / 60;
  printLCD("Paused!", String(minutes) + " mins", 2);

  if (state == WAITING) {
    currentState = WAITING;
    addStatus(currentState);
    ledcWrite(SERVO_CHN, 0);
    return;
  }

  //Keep paused
  if (hasMotions())
    return;

  //Back to running mode
  if (timePaused >= TIME_PAUSING) {
    currentState = RUNNING;
    lastDate = newDate;
    addStatus(currentState);
    return;
  }
}

void loopSystem() {
  if (lcdTime <= millis()) {
    lcd.noBacklight();
    lcdTime = ULONG_MAX;
  }

  unsigned long diff = millis() - newMilli;
  newMilli = millis();

  int speedUp = SPEED_UP_ACTING;
  if (currentState == WAITING)
    speedUp = SPEED_UP_WAITING;

  newDate += (speedUp * 60 * diff) / 1000;

  //If the system was stopped
  if (!sys["active"])
    return;

  if (currentState == RUNNING)
    running(state);
  else if (currentState == WAITING)
    waiting(state);
  else if (currentState == PAUSED)
    paused(state);
}

void addData() {
  JsonArray jsonArray;
  if (sys.containsKey("data"))
    jsonArray = sys["data"].as<JsonArray>();
  else jsonArray = sys.createNestedArray("data");

  JsonObject root = jsonArray.createNestedObject();
  root["temp"] = dht.getTemperature();
  root["hum"] = dht.getHumidity();
  root["t"] = newDate;
}

void addStatus(SystemState state) {
  JsonArray jsonArray;
  if (sys.containsKey("status"))
    jsonArray = sys["status"].as<JsonArray>();
  else jsonArray = sys.createNestedArray("status");

  JsonObject root = jsonArray.createNestedObject();
  root["status"] = state;
  root["t"] = newDate;
}
