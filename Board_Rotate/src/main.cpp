#include <Arduino.h>
#include <Servo.h>

Servo base; // Create a servo object for motor 1
Servo upm; // Create a servo object for motor 2

void setup() {
  base.attach(10);  // Attach motor 1 to pin 10
  upm.attach(11);  // Attach motor 2 to pin 11

}


void loop() {
  for(; ;){
        Serial.println("45 Horizontal Init Pos");
        base.write(135);
        upm.write(45);
        delay(500);
        Serial.println("Recyclable");
        base.write(180);
        upm.write(0); 
        delay(1500);
        base.write(135);
        upm.write(45);
        delay(500);
        Serial.println("Harmful");
        base.write(180);      
        upm.write(90); 
        delay(1500);
        base.write(135);
        upm.write(45);
        delay(500);  
        Serial.println("other");
        base.write(90);      
        upm.write(90); 
        delay(1500);
        base.write(135);
        upm.write(45);
        delay(500);        
        Serial.println("Organic");
        base.write(90);      
        upm.write(0); 
        delay(1500);
        base.write(135);
        upm.write(45);         
        delay(1500);
  }

}
