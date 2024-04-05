#include "cloud.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }

  initCloud();
}

void loop() {
}