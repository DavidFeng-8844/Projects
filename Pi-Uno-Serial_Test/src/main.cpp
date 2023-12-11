#include <Arduino.h>
#include <Servo.h>
#define TX_LED 6

const int maxDataItems = 10; // Define the maximum number of data items you expect
const int maxDataElements = 6; // Define the maximum number of elements in each data item

char dataBuffer[maxDataItems][maxDataElements][256]; // 2D array to store data


String nom = "Arduino";
String msg;
Servo base; // Create a servo object for motor 1
Servo upm; // Create a servo object for motor 2


void setup() {
  Serial.begin(9600); // Initialize serial communication
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TX_LED, OUTPUT);
  base.attach(10);  // Attach motor 1 to pin 10
  upm.attach(11);  // Attach motor 2 to pin 11

}


void loop() {
/*
  //digitalWrite(TX_LED, HIGH);
   readSerialPort();
   if (msg != "") {
    digitalWrite(TX_LED, HIGH);
    sendData();
  }
  delay(500);
        Serial.println("Hi From Arduino");
        base.write(135);
        upm.write(45);
        delay(500);*/
       
//########Version 1
        /*
if (Serial.available() > 0) {
  String data = Serial.readStringUntil('\n');
  
  // Find the index of the first opening bracket '['
  int lbracket = data.indexOf('[');

  // Find the index of the last closing bracket ']'
  int rbracket = data.lastIndexOf(']');

  if (lbracket != -1 && rbracket != -1) {
    // Extract the content within all square brackets
    String content = data.substring(lbracket + 1, rbracket);

    // Split the content into individual groups using '],[' as a separator
    String groups[10]; // Adjust the array size based on your needs
    int numGroups = content.split("],[" , groups, 10);

    for (int i = 0; i < numGroups; i++) {
      // Extract each group
      String group = groups[i];
      
      // Split the group into label and object type
      int commaIndex = group.indexOf(',');
      if (commaIndex != -1) {
        String label = group.substring(0, commaIndex);
        String objectType = group.substring(commaIndex + 2); // Skip the space after the comma

        // Process the data for each group (you can add your logic here)
        
        // Print the label and object type for each group
        Serial.print("Label: ");
        Serial.print(label);
        Serial.print("\tType: ");
        Serial.println(objectType);
      }
    }
  }
}
*/

if (Serial.available() > 0) {
    // Read data until a newline character is received
    char receivedData[256];
    int bytesRead = Serial.readBytesUntil('\n', receivedData, sizeof(receivedData) - 1);
    receivedData[bytesRead] = '\0'; // Null-terminate the string

    // Split the received data into individual lines using '\n' as a separator
    char* line = strtok(receivedData, "\n");
    int i = 0;
    int j = 0; 
    while (line != NULL && i < maxDataItems) {
      // Split each line into individual elements using ',' as a separator
      char* element = strtok(line, ",");
      j = 0; // Reset j for each line
      while (element != NULL && j < maxDataElements) {
        strcpy(dataBuffer[i][j], element);
        element = strtok(NULL, ",");
        j++;
      }
      line = strtok(NULL, "\n");
      i++;
    }

    if (i > 0 && j > 0) {
      Serial.print("First Element: ");
      Serial.println(dataBuffer[0][0]);
    }
  }


      
  //Sample Positon Dont Delete This!!!
  /*for(; ;){
        digitalWrite(LED_BUILTIN, HIGH);
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
  }*/

 /* while (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n'); // Read the incoming data

    // Parse the data and extract the garbage type
    String garbageType;
    if (Serial.read() == 's' ){
        base.write(180);
        upm.write(0); 
        delay(1500);
        base.write(135);
        upm.write(45);
        delay(500);
    }

    // Assuming data is in the format: [sequence, 'GarbageType', (motor1Angle, motor2Angle), 'confidence', 'status']
    if (sscanf(data.c_str(), "[*[^,], '%[^']', (*[^,], *[^)])", garbageType) == 1) {
      // Print the extracted garbage type
      Serial.print("Garbage Type: ");
      Serial.println(garbageType);

      // Control the motors based on the garbage type
      if (garbageType == "Recyclable") {
        base.write(180);
        upm.write(0); 
        delay(1500);
        base.write(135);
        upm.write(45);
        delay(500);
      } else if (garbageType == "Organic") {
        base.write(90);      
        upm.write(0); 
        delay(1500);
        base.write(135);
        upm.write(45);         
        delay(1500);
      } else if (garbageType == "Harmful") {
        base.write(180);      
        upm.write(90); 
        delay(1500);
        base.write(135);
        upm.write(45);
        delay(500);  
      } else if (garbageType == "Other") {
        base.write(90);      
        upm.write(90); 
        delay(1500);
        base.write(135);
        upm.write(45);
        delay(500);  
      }
    } else {
      Serial.println("Invalid data format");
    }
  }
}
void readSerialPort(){
  msg = "";
  if (Serial.available()) {
    delay(10);
    while (Serial.available() > 0) {
      msg += (char)Serial.read();
    }
    Serial.flush();
  }*/
}
/*
void readSerialPort() {
  msg = "";
  if (Serial.available()) {
    delay(10);
    while (Serial.available() > 0) {
      msg += (char)Serial.read();
    }
    Serial.flush();
  }
}

void sendData(){
  //write data
  Serial.print(nom);
  Serial.print(" received : ");
  Serial.print(msg);
}*/


//The following is the code that seperate the label and 
//types into one gorup and the coordianate into other arry
/*
if (Serial.available() > 0) {
  String data = Serial.readStringUntil('\n');
  
  // Find the index of the first opening bracket '['
  int lbracket = data.indexOf('[');

  // Find the index of the last closing bracket ']'
  int rbracket = data.lastIndexOf(']');

  if (lbracket != -1 && rbracket != -1) {
    // Extract the content within all square brackets
    String content = data.substring(lbracket + 1, rbracket);

    // Split the content into individual groups using '],[' as a separator
    String groups[10]; // Adjust the array size based on your needs
    int numGroups = content.split("],[" , groups, 10);

    for (int i = 0; i < numGroups; i++) {
      // Extract each group
      String group = groups[i];

      // Split the group into individual elements
      String elements[10]; // Adjust the array size based on your needs
      int numElements = group.split("," , elements, 10);

      if (numElements >= 3) {
        int firstElement = elements[0].toInt(); // Convert the first element to an integer
        String secondElement = elements[1].substring(2, elements[1].length() - 2); // Remove single quotes from the second element

        // Process the first and second elements (integer and string)

        // Store the pairs in an array
        int pairs[10][2]; // Adjust the array size based on your needs
        for (int j = 2; j < numElements; j++) {
          // Split each pair into two integers
          String pair = elements[j].substring(1, elements[j].length() - 1); // Remove parentheses
          int pairValues[2];
          int pairIndex = 0;
          String pairValue = "";

          for (int k = 0; k < pair.length(); k++) {
            if (pair[k] == ',') {
              pairValues[pairIndex++] = pairValue.toInt();
              pairValue = "";
            } else {
              pairValue += pair[k];
            }
          }
          pairValues[pairIndex] = pairValue.toInt();

          // Store the pair values in the array
          pairs[j - 2][0] = pairValues[0];
          pairs[j - 2][1] = pairValues[1];
        }
        
        // Now you have the first element, second element, and pairs for this group
        // You can process or store them as needed
        Serial.print("First Element: ");
        Serial.println(firstElement);
        Serial.print("Second Element: ");
        Serial.println(secondElement);
        
        // Print the pairs
        for (int j = 0; j < numElements - 2; j++) {
          Serial.print("Pair ");
          Serial.print(j + 1);
          Serial.print(": (");
          Serial.print(pairs[j][0]);
          Serial.print(", ");
          Serial.print(pairs[j][1]);
          Serial.println(")");
        }
      }
    }
  }
}
*/
