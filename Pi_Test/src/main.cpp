#include <Arduino.h>
const int stepsPerRevolution = 200;  // Change this to match your stepper motor's specifications
int motorPins[] = {8, 9, 10, 11};    // Change these to match your motor driver configuration
const int ledPin = 13;  // Pin for the built-in LED

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println("Hello David");
  delay(1500);
}

