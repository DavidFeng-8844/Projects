#include "Wire.h"
#include "MPU6050.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;

#define SAMPLE_RATE 10              /* Sample rate in ms */
#define P_P_DIFF 1000               /* Threshold value for the difference between maximum and minimum values of 3D data */ //defulat to 1000
#define FALLING_EDGE 0              /* Falling edge state */
#define FILTER_CNT 4
#define SAMPLE_SIZE 4
#define WINDOW_SIZE 300
#define MIN_WAIT 1000               /* Wait time between max and min  */
#define STEP_OK 8                   /* Number of consecutive valid steps */
#define SENSITIVITY 10000           /* Sensitivity of the sensor */
#define THRESHOLD_CNT 4
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

// The define for Oled 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


unsigned int newMax = 0, newMin = 0; /* Maximum and minimum values */
bool walkSta = false;              /* State for detecting the first step */
bool walkOkSta = false;            /* State for detecting 7 valid steps within 10s */
long lastTime = 0;                 /* Time of the last walkSta transition */
unsigned char stepOK = 0;          /* Initial step counter - reset after an invalid step */
unsigned long stepCount = 0;       /* Total step count */
static int * cur_avr_min = 0;       /* Current average minimum value */
static int * cur_avr_max = 0;       /* Current average maximum value */

// Structure definitions
typedef struct {
    short x;
    short y;
    short z;
} axis_info_t;

// struct to record the data of the filter buffer of four sets of data
typedef struct {
    axis_info_t info[FILTER_CNT]; //nested structure of type axis_info_t
    unsigned char count;
} filter_avg_t;

// define a type of 4 buffer of threshold values
typedef struct {
    axis_info_t max[THRESHOLD_CNT];
    axis_info_t min[THRESHOLD_CNT];
    unsigned char count;
    int cur_thd;
} threshold_t;

typedef struct {
    axis_info_t new_sample;
    axis_info_t old_sample;
} slid_reg_t;

// Function prototypes
static void filter_avg_init(filter_avg_t * filter);
threshold_t find_extremes(filter_avg_t * filter, axis_info_t * sample, threshold_t * threshold);
unsigned long GetTime();
int find_threshold(threshold_t * threshold);
void filter_calculate(filter_avg_t *filter, axis_info_t *sample);
int possible_step(int cur_test_thd, threshold_t * threshold, int possibleStep);
static void peak_value_init(peak_value_t * peak);
static void slid_reg_init(slid_reg_t * slid);
unsigned long Step_Count(float ax, float ay, float az);


void setup() {
    Wire.begin();
    Serial.begin(9600);

    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    pinMode(LED_BUILTIN, OUTPUT);
    
    // initialize the OLED object
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }

    // Clear the buffer.
    display.clearDisplay();
} 

void loop() {
  unsigned long currentStepCount = Step_Count(ax, ay, az); //Pass the values of the accelerometer to the step count function
  // the algorithm considers that a person is walking or running
  //if at least eight consecutive possible steps occur, which is an extra
  //measure of the algorithm to avoid false positives due to isolated
  //events
  if (currentStepCount > stepCount) {
    stepCount = currentStepCount;
    if (currentStepCount > STEP_OK) { //if the current step count is greater than 8
    if (!walkSta) {               //if the walk state is false
      walkSta = true;           //set the walk state to true
      lastTime = GetTime();     //set the last time to the current time
    }else {
      if (GetTime() - lastTime > 10000) {   //if the current time minus the last time is greater than 10 seconds
        walkSta = false;                  //set the walk state to false
      }else {
        walkSta = true;
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.print("Step Count: ");
        Serial.println(stepCount);
        Serial.println("\t\t+++++++++++++++++++++++");
        Serial.print("|\n|\n");
        Serial.print("|\tStep Count:\t");
        Serial.println(stepCount);
        Serial.println("\t\t+++++++++++++++++++++++");
        // Display the step count on the OLED screen
        display.clearDisplay();
        display.setTextSize(1); // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE); // Draw white text
        display.setCursor(0,28); // Start at top-left corner
        display.println("Step Count: ");
        display.println(stepCount);
        display.display();
      }
    }
    }else {
          walkSta = false;
    }      
  }
  else {
      digitalWrite(LED_BUILTIN, LOW);
  }
}

unsigned long GetTime() {
  return millis();
}

static void filter_avg_init(filter_avg_t *filter) {
  memset(filter->info, 0, sizeof(filter->info));
  filter->count = 0;
}

static void peak_value_init(peak_value_t *peak) {
  memset(peak, 0, sizeof(peak_value_t));
}

static void slid_reg_init(slid_reg_t *slid) {
  memset(slid, 0, sizeof(slid_reg_t));
}


// Step Count function
unsigned long Step_Count(float axis0, float axis1, float axis2) {
  static unsigned int stepCount = 0;
  static unsigned int sampleCount = 0;

  // Filter initialization
  static filter_avg_t filter;
  static peak_value_t peak;
  static slid_reg_t slid;
  static axis_info_t sample;
  static threshold_t threshold;
  static int possibleStep = 0;
  int possibleStepCount = 0;
  static int test_thd;
  int cur_test_thd;

  // Initialize structures on the first call
  if (sampleCount == 0) {
    filter_avg_init(&filter);
    peak_value_init(&peak);
    slid_reg_init(&slid);
  }

  // Find the extreme value and the current threshold
  threshold = find_extremes(&filter, &sample, &threshold);

  // Update dynamic precision
  cur_test_thd = find_threshold(&threshold);
  if (cur_test_thd != test_thd) {
    test_thd = cur_test_thd;
    Serial.println("===============");
    Serial.print("|\n|\n");
    Serial.print("|\tThreshold Upated:\t");
    Serial.println(cur_test_thd);
    Serial.println("===============");
  }
  // Detect possible steps
  // Maximum peak > (dynamic threshold + sensitivity/2)
  // Minimum peak < (dynamic threshold – sensitivity/2)
  //print the possible step count
  possibleStepCount = possible_step(test_thd, &threshold, possibleStep);
  if (possibleStepCount != possibleStep) {
    possibleStep = possibleStepCount;
    Serial.println("\t\t+++++++++++++++++++++++");
    Serial.print("|\n|\n");
    Serial.print("|\tPossible Step:\t");
    Serial.println(possibleStep);
    Serial.println("\t\t+++++++++++++++++++++++");
  }

  return possibleStep;
}

void filter_calculate(filter_avg_t *filter, axis_info_t *sample) {
  // Read sensor data and apply filter
  for(int i = 0; i < FILTER_CNT; i++) {
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    filter->info[i].x = ax;
    filter->info[i].y = ay;
    filter->info[i].z = az;
  }
  unsigned int i;
  short y_sum = 0, z_sum = 0;
  int x_sum = 0;
  for (i = 0; i < FILTER_CNT; i++) {
    x_sum += filter->info[i].x;
    y_sum += filter->info[i].y;
    z_sum += filter->info[i].z;
  }
  sample->x = x_sum / FILTER_CNT;
  sample->y = y_sum / FILTER_CNT;
  sample->z = z_sum / FILTER_CNT;
}

threshold_t find_extremes(filter_avg_t *filter, axis_info_t *sample, threshold_t *threshold) {
  unsigned long window_start_time = GetTime();
  axis_info_t max_value, min_value;
  int sp_avr_min, sp_avr_max, cur_avr_max, cur_avr_min = 0;
  bool max_found, min_found = 0;

  //Initialize max and min values
  max_value.x = max_value.y = max_value.z = 0;
  min_value.x = min_value.y = min_value.z = 0;

  while (GetTime() - window_start_time < WINDOW_SIZE) {
    // Collect a sample
    filter_calculate(filter, sample);
    // Print test sample
    Serial.print("Test Sample: ");
    Serial.print(sample->x);
    Serial.print("\t");
    Serial.print(sample->y);
    Serial.print("\t");
    Serial.println(sample->z);
    // Find the maximum value
    if (sample->x > max_value.x) max_value.x = sample->x;
    if (sample->y > max_value.y) max_value.y = sample->y;
    if (sample->z > max_value.z) max_value.z = sample->z;
    //delay(SAMPLE_RATE); // Wait for the next sample
    //Compare the average maxium
    cur_avr_max = (max_value.x + max_value.y + max_value.z) / 3;
    sp_avr_max = (sample->x + sample->y + sample->z) / 3;
    // break the loop if the current maximum value is greater than the sample maximum value
    if (sp_avr_max < cur_avr_max) {
      max_found = 1;
      threshold->max[threshold->count] = max_value;
      break;
    }
  }
  // If the maximum value is found, find the minimum value
  // Find the minimum value for up to 1 second
  if (max_found) {
    while(GetTime() - window_start_time < WINDOW_SIZE + MIN_WAIT) {
      // Collect a sample
      filter_calculate(filter, sample);
      // Find the minimum value
      if (sample->x < min_value.x) min_value.x = sample->x;
      if (sample->y < min_value.y) min_value.y = sample->y;
      if (sample->z < min_value.z) min_value.z = sample->z;
      //delay(SAMPLE_RATE); // Wait for the next sample
      //Compare the average minium
      cur_avr_min = (min_value.x + min_value.y + min_value.z) / 3;
      sp_avr_min = (sample->x + sample->y + sample->z) / 3;
      // break the loop if the current minimum value is less than the sample minimum value
      if (sp_avr_min > cur_avr_min) {
        if (threshold->count >= THRESHOLD_CNT)  {
          // Reset the threshold count
          threshold->count = 0;
          threshold->min[threshold->count] = min_value;
          min_found = 1;
          threshold->count++;
          break;
        }
        min_found = 1;
        threshold->min[threshold->count] = min_value;
        threshold->count++; //Only update the threshold count wehn the minimum value is found
        break;
      }
    }
  }
  if (min_found = 1) {
    Serial.print("Average max & min: ");
    Serial.print(cur_avr_max);
    Serial.print("\t");
    Serial.println(cur_avr_min);
  } else {
    Serial.println("No Min Value found within one second.");
  }
  return *threshold; //return the current threshold value
}

int find_threshold (threshold_t * threshold) {
  // The dynamic threshold is updated every time the difference between the
  // maximum and the minimum is greater than the SENSITIVITY.

  //Initialize the variables every time before the loop
  int sum_avr_max = 0, sum_avr_min = 0;
  int max_avr, min_avr = 0;

  //Calculate the latest average maximum and minimum values for the three axis from the threshold buffer
  for (int i = 0; i < threshold->count; i++) {
    sum_avr_max += (threshold->max[i].x + threshold->max[i].y + threshold->max[i].z) / 3;
    sum_avr_min += (threshold->min[i].x + threshold->min[i].y + threshold->min[i].z) / 3;
  }
  max_avr = sum_avr_max / threshold->count;
  min_avr = sum_avr_min / threshold->count;
  //Test print
  Serial.print("buffer max & min: ");
  Serial.print(max_avr);
  Serial.print("\t");
  Serial.println(min_avr);
  //Update the cur_thd if the difference between max_avr and min_avr is larger thatn the sensitivity
  if (max_avr - min_avr > SENSITIVITY) {
    threshold->cur_thd = (max_avr + min_avr) / 2;
  }
  return threshold->cur_thd;
}

int possible_step (int test_thd, threshold_t * threshold, int possibleStep) {
  int max_peak;
  int min_peak;

  // Compare the maximum and minimum values of the current sample with the dynamic threshold
  max_peak = (threshold->max[threshold->count].x + threshold->max[threshold->count].y + threshold->max[threshold->count].z) / 3;
  min_peak = (threshold->min[threshold->count].x + threshold->min[threshold->count].y + threshold->min[threshold->count].z) / 3;
  if (max_peak > test_thd + SENSITIVITY / 2.4 && min_peak < test_thd - SENSITIVITY / 4) {
    possibleStep++;
  }
  return possibleStep;
}

