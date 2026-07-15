/*
 * ======================================================================================================================
 *  wind.cpp - Wind Functions
 * ======================================================================================================================
 */
#include "include/output.h"
#include "include/analog.h"
#include "include/main.h"
#include "include/dfrgas.h"
#include "include/sensirion_sen66.h"
#include "include/wind.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

 /*
 * ======================================================================================================================
 *  Wind Direction - AS5600 Sensor
 * ======================================================================================================================
 */    
WIND_STR wind; 
bool      AS5600_exists     = true; // if AS5600 wind direction is found, we then will do observations
int       AS5600_ADR        = 0x36;
const int AS5600_raw_ang_hi = 0x0c;
const int AS5600_raw_ang_lo = 0x0d;

/*
 * ======================================================================================================================
 *  Wind Speed Calibration
 * ======================================================================================================================
 */
float ws_calibration = 2.64;       // From wind tunnel testing
float ws_radius = 0.079;           // In meters

/*
 * ======================================================================================================================
 *  Optipolar Hall Effect Sensor SS451A - Interrupt 1 - Aneometer
 * ======================================================================================================================
 */
volatile unsigned int anemometer_interrupt_count;
uint64_t anemometer_interrupt_stime;


/*
 * ======================================================================================================================
 *  anemometer_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
void anemometer_interrupt_handler()
{
  anemometer_interrupt_count++;
}

/* 
 *=======================================================================================================================
 * as5600_initialize() - wind direction sensor
 *=======================================================================================================================
 */
void as5600_initialize() {
  Output("AS5600:INIT");
  Wire.beginTransmission(AS5600_ADR);
  if (Wire.endTransmission()) {
    msgp = (char *) "WD:NF";
    AS5600_exists = false;
  }
  else {
    msgp = (char *) "WD:OK";

    analog_pins[2].inuse = false; // make sure we don't do A2 readings

    // Initialize Wind Speed Interrupt Based Sensor - Optipolar Hall Effect Sensor SS451A
    anemometer_interrupt_count = 0;
    anemometer_interrupt_stime = System.millis();
    attachInterrupt(ANEMOMETER_IRQ_PIN, anemometer_interrupt_handler, FALLING);
  }
  Output (msgp);
}


/* 
 *=======================================================================================================================
 * Wind_SampleDirection() -- Talk i2c to the AS5600 sensor and get direction
 *=======================================================================================================================
 */
int Wind_SampleDirection() {
  
  // Read Raw Angle Low Byte
  Wire.beginTransmission(AS5600_ADR);
  Wire.write(AS5600_raw_ang_lo);
  if (Wire.endTransmission()) {
    if (AS5600_exists) {
      Output ("WD Offline_L");
    }
    AS5600_exists = false;
  }
  else if (Wire.requestFrom(AS5600_ADR, 1)) {
    int AS5600_lo_raw = Wire.read();
  
    // Read Raw Angle High Byte
    Wire.beginTransmission(AS5600_ADR);
    Wire.write(AS5600_raw_ang_hi);
    if (Wire.endTransmission()) {
      if (AS5600_exists) {
        Output ("WD Offline_H");
      }
      AS5600_exists = false;
    }
    else if (Wire.requestFrom(AS5600_ADR, 1)) {
      word AS5600_hi_raw = Wire.read();

      if (!AS5600_exists) {
        Output ("WD Online");
      }
      AS5600_exists = true;           // We made it       
      AS5600_hi_raw = AS5600_hi_raw << 8; //shift raw angle hi 8 left
      AS5600_hi_raw = AS5600_hi_raw | AS5600_lo_raw; //AND high and low raw angle value
      return ((int) AS5600_hi_raw *0.0879);
    }
  }
  return (-1); // Not the best value to return 
}

/* 
 *=======================================================================================================================
 * Wind_SampleSpeed() - Return a wind speed based on interrupts and duration wind
 * 
 * Optipolar Hall Effect Sensor SS451A - Aneometer
 * speed  = (( (signals/2) * (2 * pi * radius) ) / time) * calibration_factor
 * speed in m/s =  (   ( (interrupts/2) * (2 * 3.14156 * 0.079) )  / (time_period in ms / 1000)  )  * 2.64
 *=======================================================================================================================
 */
float Wind_SampleSpeed() {
  uint64_t delta_ms;
  float wind_speed;
  
  delta_ms = System.millis()-anemometer_interrupt_stime;

  if (anemometer_interrupt_count) {
    // wind_speed = (  ( (anemometer_interrupt_count/2) * (2 * 3.14156 * ws_radius) )  / 
    //  (float)( (float)delta_ms / 1000)  )  * ws_calibration;

    wind_speed = ( ( anemometer_interrupt_count * 3.14156 * ws_radius)  / 
        (float)( (float)delta_ms / 1000) )  * ws_calibration;
  }
  else {
    wind_speed = 0.0;
  }

  anemometer_interrupt_count = 0;
  anemometer_interrupt_stime = System.millis();
  
  return (wind_speed);
} 

/*
 * ======================================================================================================================
 * Wind_TakeReading() - Wind direction and speed, measure every second             
 * ======================================================================================================================
 */
void Wind_TakeReading() {
  if (AS5600_exists) {
    wind.bucket[wind.bucket_idx].direction = (int) Wind_SampleDirection();
    wind.bucket[wind.bucket_idx].speed = Wind_SampleSpeed();
    wind.bucket_idx = (++wind.bucket_idx) % WIND_READINGS; // Advance bucket index for next reading
  }
}

/* 
 *=======================================================================================================================
 * Wind_DirectionVector()
 *=======================================================================================================================
 */
int Wind_DirectionVector() {
  double NS_vector_sum = 0.0;
  double EW_vector_sum = 0.0;
  double r;
  float s;
  int d, i, rtod;
  bool ws_zero = true;

  for (i=0; i<WIND_READINGS; i++) {
    d = wind.bucket[i].direction;

    // if at any time 1 of the 60 wind direction readings is -1
    // then the sensor was offline and we need to invalidate or data
    // until it is clean with out any -1's
    if (d == -1) {
      return (-1);
    }
    
    s = wind.bucket[i].speed;

    // Flag we have wind speed
    if (s > 0) {
      ws_zero = false;  
    }
    r = (d * 71) / 4068.0;
    
    // North South Direction 
    NS_vector_sum += cos(r) * s;
    EW_vector_sum += sin(r) * s;
  }
  rtod = (atan2(EW_vector_sum, NS_vector_sum)*4068.0)/71.0;
  if (rtod<0) {
    rtod = 360 + rtod;
  }

  // If all the winds speeds are 0 then we return current wind direction or 0 on failure of that.
  if (ws_zero) {
    return (Wind_SampleDirection()); // Can return -1
  }
  else {
    return (rtod);
  }
}

/* 
 *=======================================================================================================================
 * Wind_SpeedAverage()
 *=======================================================================================================================
 */
float Wind_SpeedAverage() {
  float wind_speed = 0.0;
  for (int i=0; i<WIND_READINGS; i++) {
    // sum wind speeds for later average
    wind_speed += wind.bucket[i].speed;
  }
  return( wind_speed / (float) WIND_READINGS);
}

/* 
 *=======================================================================================================================
 * Wind_Gust()
 *=======================================================================================================================
 */
float Wind_Gust() {
  return(wind.gust);
}

/* 
 *=======================================================================================================================
 * Wind_GustDirection()
 *=======================================================================================================================
 */
int Wind_GustDirection() {
  return(wind.gust_direction);
}

/* 
 *=======================================================================================================================
 * Wind_GustUpdate()
 *   Wind Gust = Highest 3 consecutive samples from the 60 samples. The 3 samples are then averaged.
 *   Wind Gust Direction = Average of the 3 Vectors from the Wind Gust samples.
 * 
 *   Note: To handle the case of 2 or more gusts at the same speed but different directions
 *          Sstart with oldest reading and work forward to report most recent.
 * 
 *   Algorithm: 
 *     Start with oldest reading.
 *     Sum this reading with next 2.
 *     If greater than last, update last 
 * 
 *=======================================================================================================================
 */
void Wind_GustUpdate() {
  int bucket = wind.bucket_idx; // Start at next bucket to fill (aka oldest reading)
  float ws_sum = 0.0;
  int ws_bucket = bucket;
  float sum;

  for (int i=0; i<(WIND_READINGS-2); i++) {  // subtract 2 because we are looking ahead at the next 2 buckets
    // sum wind speeds 
    sum = wind.bucket[bucket].speed +
          wind.bucket[(bucket+1) % WIND_READINGS].speed +
          wind.bucket[(bucket+2) % WIND_READINGS].speed;
    if (sum >= ws_sum) {
      ws_sum = sum;
      ws_bucket = bucket;
    }
    bucket = (bucket+1) % WIND_READINGS;
  }
  wind.gust = ws_sum/3;
  
  // Determine Gust Direction 
  double NS_vector_sum = 0.0;
  double EW_vector_sum = 0.0;
  double r;
  float s;
  int d, i, rtod;
  bool ws_zero = true;

  bucket = ws_bucket;
  for (i=0; i<3; i++) {
    d = wind.bucket[bucket].direction;

    // if at any time any wind direction readings is -1
    // then the sensor was offline and we need to invalidate or data
    // until it is clean with out any -1's
    if (d == -1) {
      ws_zero = true;
      break;
    }
    
    s = wind.bucket[bucket].speed;

    // Flag we have wind speed
    if (s > 0) {
      ws_zero = false;  
    }
    r = (d * 71) / 4068.0;
    
    // North South Direction 
    NS_vector_sum += cos(r) * s;
    EW_vector_sum += sin(r) * s;

    bucket = (bucket+1) % WIND_READINGS;
  }

  rtod = (atan2(EW_vector_sum, NS_vector_sum)*4068.0)/71.0;
  if (rtod<0) {
    rtod = 360 + rtod;
  }

  // If all the winds speeds are 0 or we has a -1 direction then set -1 dor direction.
  if (ws_zero) {
    wind.gust_direction = -1;
  }
  else {
    wind.gust_direction = rtod;
  }
}

/* 
 *=======================================================================================================================
 * Wind_Fill()
 *=======================================================================================================================
 */
void Wind_Fill() {
  if (AS5600_exists) {
    Output("Wind Fill");

    // Clear windspeed counter  
    anemometer_interrupt_count = 0;
    anemometer_interrupt_stime = System.millis();
  
    // Init default values.
    wind.gust = 0.0;
    wind.gust_direction = -1;
    wind.bucket_idx = 0;
  
    for (int i=0; i<WIND_READINGS; i++) {
      wind.bucket[i].direction = (int) -999;
      wind.bucket[i].speed = 0.0;
    }

    for (int i=0; i<WIND_READINGS; i++) {
      if (AS5600_exists) {
        Wind_TakeReading();
        float ws = Wind_SpeedAverage();
        sprintf (Buffer32Bytes, "%02d WD:%3d WS:%d.%02d", 
          i, Wind_SampleDirection(), (int)ws, (int)(ws*100)%100);
        Output(Buffer32Bytes);
      }
      delay (990);  // Substract a little time from 750 for loop execution (This is a guess)
    }
  }
}

/* 
 *=======================================================================================================================
 * Fill_WindGas()
 *=======================================================================================================================
 */
void Fill_WindGas() {

  if ((dfrgas_sp->number_found == 0) && (sen66_sp->number_found == 0) && !AS5600_exists) {
    Output("No Wind, Gas or AQ to Fill");
    return;
  }
  Output("Fill Wind&Gas");

  if (AS5600_exists) {
    // Clear windspeed counter  
    anemometer_interrupt_count = 0;
    anemometer_interrupt_stime = System.millis();
  
    // Init default values.
    wind.gust = 0.0;
    wind.gust_direction = -1;
    wind.bucket_idx = 0;
  
    for (int i=0; i<WIND_READINGS; i++) {  
      wind.bucket[i].direction = (int) -999;
      wind.bucket[i].speed = 0.0;
    }
  }

  uint64_t OneSecondFromNow, TimeRemaining;

  for (int i=0; i<60; i++) {// WIND_READINGS & DFRGAS_READINGS both equal 60
    OneSecondFromNow = System.millis() + 1000;

    if (AS5600_exists) {
      Wind_TakeReading();
      float ws = Wind_SpeedAverage();
      sprintf (Buffer32Bytes, "%02d WD:%3d WS:%d.%02d", 
        i, Wind_SampleDirection(), (int)ws, (int)(ws*100)%100);
      Output(Buffer32Bytes);
    }
    if (dfrgas_sp->number_found) {
      dfrgas_TakeReading();
      Output ("DFRGAS Take Reading");
    }
    if (sen66_sp->number_found) {
      sen66_TakeReading();
      Output ("SEN66 Take Reading");
    }
    
    TimeRemaining = (OneSecondFromNow - System.millis());
    if ((TimeRemaining > 0) && (TimeRemaining < 1000)) {
      delay (TimeRemaining);
    }
  }
}