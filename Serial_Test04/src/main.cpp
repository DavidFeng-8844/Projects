#include <Arduino.h>


void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
      String response = Serial.readString();
      Serial.println((String)response);

  }
  }

