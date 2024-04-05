#include "cloud.h"

const char* SSID = "MEO-6B144F";
const char* PASSWORD = "CA22156F4B";

const char* URL = "http://85.244.117.107";
const char* END_POINT_GET = "/test/McMacau";
const char* END_POINT_POST = "/test";

const int WIFI_CONNECTION_DELAY = 500;

void handleGet(DynamicJsonDocument json) {
  String dataValue = json["result"]["uuid"];
  Serial.print("UUID: ");
  Serial.println(dataValue);
}

/*
* Starts the wifi conenction
*/
void initCloud() {

  Serial.println("\n\n\n");
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(WIFI_CONNECTION_DELAY);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");

  //Get request
  Serial.println("\n\nPerform GET");
  getRequest(END_POINT_GET, handleGet);

  //Post request
  Serial.println("\n\nPerform POST");
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["uuid"] = "1212";
  root["username"] = "daniel";
  root["password"] = "pwd32132";
  root["ip"] = "23.212.213.21";
  postRequest(END_POINT_POST, root, handleGet);
  
}

/*
* Create a get request to the cloud
*
* const char* endpoint -> request endpoint
* fetchFunction fun    -> function to handle the data returned
*/
void getRequest(const char* endpoint, fetchFunction fun) {
  if (WiFi.status() != WL_CONNECTED)
    return;

  String requestURL = String(URL) + endpoint;
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
* const char* endpoint -> request endpoint
* JsonObject& obj      -> json object to be sent
* fetchFunction fun    -> function to handle the data returned
*/
void postRequest(const char* endpoint, JsonObject& obj, fetchFunction fun) {
  if (WiFi.status() != WL_CONNECTED)
    return;

  String requestURL = String(URL) + endpoint;
  HTTPClient http;
  http.begin(requestURL);
  http.addHeader("Content-Type", "application/json");

  String jsonString;
  serializeJson(obj, jsonString);
  int httpResponseCode = http.POST(jsonString);

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
