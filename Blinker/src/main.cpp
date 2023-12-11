#include <Arduino.h>
#define TX_LED 6
#define RX_LED 5

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RX_LED, OUTPUT);
  pinMode(TX_LED, OUTPUT);    
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(RX_LED, HIGH);
  digitalWrite(TX_LED, HIGH);    
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(RX_LED, LOW);
  digitalWrite(TX_LED, LOW); 
  delay(200);
  myFunction(18,44);
  
}
 
// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
  Serial.print("xy");

}