/*
 * ======================================================================================================================
 *  dfrwrg.h - DFRobot Gravity: HX711 Weight Sensor - Weighing Rain Gauge
 * 
 * SEE
 *   https://wiki.dfrobot.com/kit0176/
 *   https://wiki.dfrobot.com/kit0176/docs/21753
 *   Library https://github.com/dfrobot/DFRobot_HX711_I2C 
 * 
 * Reported tags name examples wrg1 where 1 is the id you specifed in mux_sensor_config()
 * 
 * 1g of water is approximately 1 cm^3
 * Since 1 cm = 10 mm:
 * depth_mm = waterWeight_g * 10 / collector_area_cm^2
 * 
 * ======================================================================================================================
 */

 /*
 * Why Calibration Is Required When Using a Baseline
 *
 * A baseline and calibration serve different purposes:
 *
 *   - Calibration establishes the relationship between the load-cell
 *     signal and actual weight in grams.
 *
 *   - The baseline establishes the application's starting condition and
 *     removes the initial zero offset.
 *
 * The application calculates the change as:
 *
 *     change_g = currentWeight_g - baselineWeight_g;
 *
 */

 /*
 * CAL button: performs an automatic single-point calibration using a known weight.
 * setCalibration(value): manually loads a calibration coefficient that you already know; it does not independently calibrate the sensor.
 * 
 * 
 * CAL Button Calibration Procedure
 *
 * The CAL button should not be used with an empty sensor when performing
 * normal calibration.
 *
 * An empty platform establishes the zero point, or tare, but calibration
 * also requires a known reference weight to determine the grams-per-count
 * scale factor.
 * 
 * You should calibrate the sensor after it is mechanically installed in the finished product, 
 * using the same platform, tray, mounting hardware, and orientation used during normal operation.
 *
 * Recommended procedure:
 *   0. Load the calibration test program, which is modified to set the cal value and report weight after set
 *   1. Remove the load, but leave the permanent platform or tray installed.
 *   2. Start calibration test program. Led is on, add the weight
 *   3. Led blinks 3 times
 *   4. Verify that the empty platform reads approximately zero.
 *
 *
 */

 /*
 *
 * A tare or baseline removes the zero offset, but it does not determine
 * or verify the scale factor needed for accurate readings in grams.
 * 
 * Tare means setting the scale’s current reading to zero without changing the calibration scale factor.
 * peel() is DFRobot’s library function that performs this tare operation on the KIT0176. It “peels off” the current weight from subsequent measurements.
 * 
 * First calibrate the sensor with a known mass.
 * Then place the empty container or platform on the scale.
 *    scale.peel();  // Set the current condition to zero.
 * Future readings represent material added after the tare.
 *    float materialWeight_g = scale.readWeight(12);
 */

#include <DFRobot_HX711_I2C.h>
#include "include/sensors.h"

#define WRG_READINGS          60        // Maintain 60 1 second samples
#define WRG_TIP_DROP_G        100.0f    // Minimum drop to consider a tip
#define WRG_NOISE_FLOOR_G     0.5f      // Minimum change to treat as real weight change
#define WRG_MIN_G             0.0f      // Minimum sanity check
#define WRG_MAX_G             1000.0f   // Maximum sanity check

// CO Sensor Range is 0-1000 PPM

#define DFRWRG_MUX_CHANNELS 8

typedef struct {
  float baseline_sample;
  float last_sample;
  float rain_slr; // since last reported
  float calibration;
  float bucket[WRG_READINGS];
} DFRGRAV_SENSOR;

typedef struct {
  DFRGRAV_SENSOR sensor[DFRWRG_MUX_CHANNELS];
  int number_found;
  int bucket_idx;
} DFRGRAV_SENSORS;

// Extern variables
extern DFRGRAV_SENSORS dfrwrg_sensors;
extern DFRGRAV_SENSORS *dfrwrg_sp;
extern DFRobot_HX711_I2C *dfrwrg;    // All the Sensors mush have the same i2c address, so we are supporting 1 per mux channel


// Function prototype
void dfrwrg_setup();
void dfrwrg_init(DFRobot_HX711_I2C *&dfrwrg, CH_SENSOR *chs, int mux_channel);
void dfrwrg_TipCheck();
float dfrwrg_read(int mux_channel);
float dfrwrg_Median(int c);
void dfrwrg_TakeReading();