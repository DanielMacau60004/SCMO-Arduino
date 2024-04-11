#include "cloud.h"

const char* URL = "http://85.244.117.107";
const char* END_POINT_GET = "/test/McMacau";
const char* END_POINT_POST = "/test";

const char* URL_DATETIME_UTC = "https://worldtimeapi.org/api/timezone/utc";

unsigned long currentDate = 0;

// Temporary method...
/*void initCloud() {

  //Get request
  Serial.println("\nPerform GET");
  getRequest(END_POINT_GET, handleGet);

  //Post request
  Serial.println("Perform POST");
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["uuid"] = "1212";
  root["username"] = "daniel";
  root["password"] = "pwd32132";
  root["ip"] = "23.212.213.21";
  postRequest(END_POINT_POST, root, handleGet);
  
}*/

/*
* Create a get request to the cloud
*
* const char* url -> request url
* fetchFunction fun    -> function to handle the data returned
*/
void getRequest(const char* url, fetchFunction fun) {
  if (WiFi.status() != WL_CONNECTED)
    return;

  String requestURL = url;
  HTTPClient http;
  http.begin(requestURL);

  int httpResponseCode = http.GET();
  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();
    int responseLength = http.getSize();

    DynamicJsonDocument jsonBuffer(responseLength + 1);
    DeserializationError error = deserializeJson(jsonBuffer, response);

    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      return;
    }

    fun(jsonBuffer);
  }

  http.end();
}

/*
* Create a post request to the cloud
*
* const char* url -> request url
* JsonObject& obj      -> json object to be sent
* fetchFunction fun    -> function to handle the data returned
*/
void postRequest(const char* url, JsonObject& obj, fetchFunction fun) {
  if (WiFi.status() != WL_CONNECTED)
    return;

  String requestURL = url;
  HTTPClient http;
  http.begin(requestURL);
  http.addHeader("Content-Type", "application/json");

  String jsonString;
  serializeJson(obj, jsonString);
  int httpResponseCode = http.POST(jsonString);
  Serial.println(jsonString);
  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();
    int responseLength = http.getSize();

    DynamicJsonDocument jsonBuffer(responseLength + 1);
    DeserializationError error = deserializeJson(jsonBuffer, response);

    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      return;
    }

    fun(jsonBuffer);
  }

  http.end();
}

void updateCurrentDate(DynamicJsonDocument json) {
  unsigned long date = json["unixtime"].as<unsigned long>();
  currentDate = date - millis() / 1000;
}

void fetchCurrentDate() {
  getRequest(URL_DATETIME_UTC, updateCurrentDate);
}

unsigned long getCurrentDate() {
  return currentDate + millis() / 1000;
}
