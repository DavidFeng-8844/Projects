#include "SevSeg.h"
SevSeg sevseg; //Initiate a seven segment controller object


void setup() {
    byte numDigits = 4;  
    byte digitPins[] = {2, 3, 4, 5};
    byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
    bool resistorsOnSegments = 0; 
    // variable above indicates that 4 resistors were placed on the digit pins.
    // set variable to 1 if you want to use 8 resistors on the segment pins.
    sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins, resistorsOnSegments);
    sevseg.setBrightness(90);
}
void loop() {
    
  //  delay(200);
    sevseg.setNumber(1844);
   // delay(900);
    sevseg.refreshDisplay(); // Must run repeatedly

    
}