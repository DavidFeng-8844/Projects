#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define MPU6050_ADDRESS 0x68
#define SSD1306_I2C_ADDRESS 0x3C

int16_t accelX, accelY, accelZ;
int stepCounter = 0;
bool ledState = LOW;

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void MPU6050_Init() {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // Clear the sleep bit (wakes up the MPU-6050)
  Wire.endTransmission(true);
}

void MPU6050_ReadAccelerometer(int16_t& accelX, int16_t& accelY, int16_t& accelZ) {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x3B);  // Starting register for accelerometer data
  Wire.endTransmission(false);
  Wire.requestFrom(static_cast<uint8_t>(MPU6050_ADDRESS), 6, true);

  accelX = Wire.read() << 8 | Wire.read();
  accelY = Wire.read() << 8 | Wire.read();
  accelZ = Wire.read() << 8 | Wire.read();
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(LED_BUILTIN, OUTPUT);

  // OLED initialization
  if (!display.begin(SSD1306_I2C_ADDRESS, 4, 5)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);  // Pause for 2 seconds
  display.clearDisplay();

  MPU6050_Init();
}

void loop() {
  // Read accelerometer data
  MPU6050_ReadAccelerometer(accelX, accelY, accelZ);

  // Simple step detection based on acceleration threshold
  if (accelZ > 15000) {
    stepCounter++;
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Steps: ");
    display.print(stepCounter);
    display.display();
  }

  // Blink the built-in LED
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState);

  delay(100);  // Adjust the delay as needed for your application
}
