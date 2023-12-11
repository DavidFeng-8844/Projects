#include "Wire.h"
#include "MPU6050.h"
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;

#define SAMPLE_RATE 10              /* Sample rate in ms */
#define OUTPUT_FILTER_SAMPLE 0      /* Output filtered sample */
#define P_P_DIFF 1000               /* Threshold value for the difference between maximum and minimum values of 3D data */ //defulat to 1000
#define RISING_EDGE 1               /* Rising edge state */
#define FALLING_EDGE 0              /* Falling edge state */
#define FILTER_CNT 4
#define DYNAMIC_PRECISION 30
#define ACTIVE_PRECISION 60
#define SAMPLE_SIZE 4
#define WINDOW_SIZE 300
#define MOST_ACTIVE_NULL 0
#define MOST_ACTIVE_X 1
#define MOST_ACTIVE_Y 2
#define MOST_ACTIVE_Z 3
#define FAST_WALK_TIME_LIMIT_MS 200 /* ms */
#define SLOW_WALK_TIME_LIMIT_MS 10000 /* 10s - time without a valid step to reset step count */
#define STEP_OK 8                   /* Number of consecutive valid steps */
#define MIN_WAIT 1000               /* Wait time between max and min  */

unsigned int lastPos = 0;         /* Previous position */
unsigned int newMax = 0, newMin = 0; /* Maximum and minimum values */
bool walkSta = false;              /* State for detecting the first step */
bool walkOkSta = false;            /* State for detecting 7 valid steps within 10s */
bool pSta = RISING_EDGE;           /* 3D data state */
long lastTime = 0;                 /* Time of the last walkSta transition */
unsigned char stepOK = 0;          /* Initial step counter - reset after an invalid step */
unsigned long stepCount = 0;       /* Total step count */

// Structure definitions
typedef struct {
    short x;
    short y;
    short z;
} axis_info_t;

typedef struct {
    axis_info_t newmax;
    axis_info_t newmin;
    axis_info_t oldmax;
    axis_info_t oldmin;
} peak_value_t;

typedef struct {
    axis_info_t new_sample;
    axis_info_t old_sample;
} slid_reg_t;

// struct to record the data of the filter buffer of four sets of data
typedef struct {
    axis_info_t info[FILTER_CNT]; //nested structure of type axis_info_t
    unsigned char count;
} filter_avg_t;

// Function prototypes
static void filter_avg_init(filter_avg_t *filter);
static void peak_value_init(peak_value_t *peak);
static void slid_reg_init(slid_reg_t *slid);
unsigned long Step_Count(float ax, float ay, float az);
void find_extremes(filter_avg_t *filter, axis_info_t *sample);
unsigned long GetTime();

void setup() {
    Wire.begin();
    Serial.begin(9600);

    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  unsigned long currentStepCount = Step_Count(ax, ay, az); //Pass the values of the accelerometer to the step count function
  //Serial.println(currentStepCount);
  if (currentStepCount > stepCount) {
      // Incremented step count
      stepCount = currentStepCount;
      //Serial.print("Step Count: ");
      //Serial.println(stepCount);
      digitalWrite(LED_BUILTIN, HIGH);
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
// Add these functions'prototypes at the beginning of your sketch
void filter_calculate(filter_avg_t *filter, axis_info_t *sample);
void peak_update(peak_value_t *peak, axis_info_t *cur_sample);
char slid_update(slid_reg_t *slid, axis_info_t *cur_sample);
char is_most_active(peak_value_t *peak);
void detect_step(peak_value_t *peak, slid_reg_t *slid, axis_info_t *cur_sample, unsigned int *stepCount);


unsigned long Step_Count(float axis0, float axis1, float axis2) {
  static unsigned int stepCount = 0;
  static unsigned int sampleCount = 0;

  // Filter initialization
  static filter_avg_t filter;
  static peak_value_t peak;
  static slid_reg_t slid;
  static axis_info_t sample;

  // Initialize structures on the first call
  if (sampleCount == 0) {
    filter_avg_init(&filter);
    peak_value_init(&peak);
    slid_reg_init(&slid);
  }

  // Update dynamic threshold
  //peak_update(&peak, &filteredSample);

  // Find the extreme value
  find_extremes(&filter, &sample);

  // Update dynamic precision
  //char slidUpdated = slid_update(&slid, &filteredSample);

  // Detect steps
  detect_step(&peak, &slid, &sample, &stepCount);

  // Increment sample count
  sampleCount++;

  return stepCount;
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

void find_extremes(filter_avg_t *filter, axis_info_t *sample) {
  unsigned long window_start_time = GetTime();
  axis_info_t max_value, min_value;
  int cur_avr_min, sp_avr_min, cur_avr_max, sp_avr_max = 0;
  bool max_found, min_found = 0;

  //Initialize max and min values
  max_value.x = max_value.y = max_value.z = 0;
  min_value.x = min_value.y = min_value.z = 0;

  while (GetTime() - window_start_time < WINDOW_SIZE) {
    // Collect a sample
    filter_calculate(filter, sample);
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
      cur_avr_min = (max_value.x + max_value.y + max_value.z) / 3;
      sp_avr_min = (sample->x + sample->y + sample->z) / 3;
      // break the loop if the current minimum value is less than the sample minimum value
      if (sp_avr_min > cur_avr_min) {
        min_found = 1;
        break;
      }
    }
  }
  if (min_found = 1) {
    //Serial.println("Min Value found within one second!");
    Serial.print("Average Min Value: ");
    Serial.println(cur_avr_min);
  } else {
    Serial.println("No Min Value found within one second.");
  }


}
void peak_update(peak_value_t *peak, axis_info_t *cur_sample) {
  static unsigned int sampleSize = 0;
  sampleSize++;

  if (sampleSize > SAMPLE_SIZE) {
    sampleSize = 1;
    peak->oldmax = peak->newmax;
    peak->oldmin = peak->newmin;
    peak_value_init(peak);
  }

  peak->newmax.x = max(peak->newmax.x, cur_sample->x);
  peak->newmax.y = max(peak->newmax.y, cur_sample->y);
  peak->newmax.z = max(peak->newmax.z, cur_sample->z);

  peak->newmin.x = min(peak->newmin.x, cur_sample->x);
  peak->newmin.y = min(peak->newmin.y, cur_sample->y);
  peak->newmin.z = min(peak->newmin.z, cur_sample->z);
}

char slid_update(slid_reg_t *slid, axis_info_t *cur_sample) {
  char res = 0;

  if (abs((cur_sample->x - slid->new_sample.x)) > DYNAMIC_PRECISION) {
    slid->old_sample.x = slid->new_sample.x;
    slid->new_sample.x = cur_sample->x;
    res = 1;
  } else {
    slid->old_sample.x = slid->new_sample.x;
  }

  if (abs((cur_sample->y - slid->new_sample.y)) > DYNAMIC_PRECISION) {
    slid->old_sample.y = slid->new_sample.y;
    slid->new_sample.y = cur_sample->y;
    res = 1;
  } else {
    slid->old_sample.y = slid->new_sample.y;
  }

  if (abs((cur_sample->z - slid->new_sample.z)) > DYNAMIC_PRECISION) {
    slid->old_sample.z = slid->new_sample.z;
    slid->new_sample.z = cur_sample->z;
    res = 1;
  } else {
    slid->old_sample.z = slid->new_sample.z;
  }

  return res;
}

char is_most_active(peak_value_t *peak) {
  char res = MOST_ACTIVE_NULL;
  short x_change = abs((peak->newmax.x - peak->newmin.x));
  short y_change = abs((peak->newmax.y - peak->newmin.y));
  short z_change = abs((peak->newmax.z - peak->newmin.z));

  if (x_change > y_change && x_change > z_change && x_change >= ACTIVE_PRECISION) {
    res = MOST_ACTIVE_X;
  } else if (y_change > x_change && y_change > z_change && y_change >= ACTIVE_PRECISION) {
    res = MOST_ACTIVE_Y;
  } else if (z_change > x_change && z_change > y_change && z_change >= ACTIVE_PRECISION) {
    res = MOST_ACTIVE_Z;
  }

  return res;
}

void detect_step(peak_value_t *peak, slid_reg_t *slid, axis_info_t *cur_sample, unsigned int *stepCount) {
  static unsigned int stepCnt = 0;
  // step detection logic
  char active_axis = is_most_active(peak);
  unsigned long currentTime = GetTime();

  if (active_axis == MOST_ACTIVE_X) {
    if (cur_sample->x > slid->old_sample.x && pSta == RISING_EDGE) {
      pSta = FALLING_EDGE;
      lastTime = currentTime;
      newMin = slid->old_sample.x;
    } else if (cur_sample->x < slid->old_sample.x && pSta == FALLING_EDGE) {
      pSta = RISING_EDGE;
      newMax = slid->old_sample.x;
    }
  } else if (active_axis == MOST_ACTIVE_Y) {
    if (cur_sample->y > slid->old_sample.y && pSta == RISING_EDGE) {
      pSta = FALLING_EDGE;
      lastTime = currentTime;
      newMin = slid->old_sample.y;
    } else if (cur_sample->y < slid->old_sample.y && pSta == FALLING_EDGE) {
      pSta = RISING_EDGE;
      newMax = slid->old_sample.y;
    }
  } else if (active_axis == MOST_ACTIVE_Z) {
    if (cur_sample->z > slid->old_sample.z && pSta == RISING_EDGE) {
      pSta = FALLING_EDGE;
      lastTime = currentTime;
      newMin = slid->old_sample.z;
    } else if (cur_sample->z < slid->old_sample.z && pSta == FALLING_EDGE) {
      pSta = RISING_EDGE;
      newMax = slid->old_sample.z;
    }
  }

  if (pSta == RISING_EDGE) {
    if (newMax - newMin > P_P_DIFF) {
      if (walkSta == false) {
        walkSta = true;
        stepCnt = 0;
      }

      if (walkOkSta == false) {
        walkOkSta = true;
        lastTime = currentTime;
        stepCnt++;
      } else {
        if ((currentTime - lastTime) >= FAST_WALK_TIME_LIMIT_MS) {
          if (stepCnt < STEP_OK) {
            stepCnt = 0;
            walkOkSta = false;
          }
        } else {
          stepCnt++;
        }
      }
    }
  }

  if ((currentTime - lastTime) >= SLOW_WALK_TIME_LIMIT_MS) {
    stepCnt = 0;
    walkOkSta = false;
  }

  if (walkSta == true && walkOkSta == true && stepCnt >= STEP_OK) {
    walkSta = false;
    walkOkSta = false;
    stepCnt = 0;
    *stepCount += 1;
  }

}
