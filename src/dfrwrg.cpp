/*
 * ======================================================================================================================
 *  dfrwrg.cpp - DFRobot Gravity: HX711 Weight Sensor - Weighing Rain Gauge
 * ======================================================================================================================
 */
#include "include/output.h"
#include "include/analog.h"
#include "include/main.h"
#include "include/sensors.h"
#include "include/dfrwrg.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
DFRobot_HX711_I2C *dfrwrg = nullptr;

 /*
 * ======================================================================================================================
 *  DFRobot GAS Sensor
 * ======================================================================================================================
 */
DFRGRAV_SENSORS dfrwrg_sensors;
DFRGRAV_SENSORS *dfrwrg_sp;

#ifdef NOWAY

float gramsToMillimeters(float waterWeight_g)
{
  /*
   * 1 g of water is approximately 1 cm^3.
   *
   * rainfall_mm = waterWeight_g * 10 / collector_area_cm2
   */
  return; // (waterWeight_g * 10.0f) / COLLECTOR_AREA_CM2;
}

#endif

/* 
 *=======================================================================================================================
 * dfrwrg_read - return rain amount accumulated since last called.
 *=======================================================================================================================
 */
float dfrwrg_read(int mux_channel) {
  float reading = dfrwrg->readWeight();

  if (reading > dfrwrg_sp->sensor[mux_channel].baseline_sample) {
    // Add what we collected to amounts from prior tips since last report
    dfrwrg_sp->sensor[mux_channel].rain_slr += (reading - dfrwrg_sp->sensor[mux_channel].baseline_sample);
  }

  float rain = dfrwrg_sp->sensor[mux_channel].rain_slr; // rain to be reported

  // clear rain accumulation
  dfrwrg_sp->sensor[mux_channel].baseline_sample = reading;  // baseline is now what ever the sensor is reading.
  dfrwrg_sp->sensor[mux_channel].last_sample = reading;
  dfrwrg_sp->sensor[mux_channel].rain_slr = 0;

  return (rain);
}

/* 
 *=======================================================================================================================
 * dfrwrg_TipCheck() -- run through all sensors looking for a tip
 * 
 * Called once per second to monitor a weighing rain gauge (WRG) and detect bucket tips.
 * For each online WRG sensor:
 *   - Reads the current weight from the scale.
 *   - If the weight drops by more than ~10 g compared to the last sample, a tip is assumed.
 *     The rain accumulated since the last baseline is added to the running total (rain_slr),
 *     and both baseline_sample and last_sample are reset to the new (post-tip) weight.
 *   - If the weight increases, small changes (< 0.5 g) are treated as scale jitter and ignored.
 *     Increases >= 0.5 g are logged as added weight and update last_sample.
 *
 * Assumptions:
 *   - A genuine tip always reduces the measured weight by at least 10 g beyond normal variance.
 *   - Normal scale noise / short-term jitter is <= 0.5 g between 1-second samples.
 *   - The bucket empties cleanly on each tip, so the post-tip weight can be used as the new baseline.
 *   - Negative weight readings are invalid and cause this call to exit early.
 *=======================================================================================================================
 */
void dfrwrg_TipCheck() {

  if (!dfrwrg_sp->number_found) {
    return;
  }

  for (int c=0; c<MUX_CHANNELS; c++) {
    mc = &mux[c];
    if (mc->inuse) {
      // Loop through sensors on channel
      for (int s=0; s<MAX_CHANNEL_SENSORS; s++) {
        chs = &mc->sensor[s];
        if ((chs->type != wrg) | (chs->state != ONLINE)) {
          continue;
        }

        mux_channel_set(c); // Set mux channel

        float reading = dfrwrg->readWeight();
        if (reading < WRG_MIN_G || reading > WRG_MAX_G) {
          // Log warning; skip this sample
          continue;
        }

        float last = dfrwrg_sp->sensor[c].last_sample;
        float baseline = dfrwrg_sp->sensor[c].baseline_sample;
        float delta = reading - last;

        // so if I add a weight on that is 100g, but it take 2 readings for it to settle
        // 1st = 87 and 2nd = 13.  The is change from the last reading is seen as a tip

        if (delta <= -WRG_TIP_DROP_G) {  // Delta can be negative, if negative by more than -10 g we tipped
      
          float old_slr = dfrwrg_sp->sensor[c].rain_slr; // since last report
          float added = 0.0f;

          if (last > baseline) {
            added = last - baseline;
          }
          float new_total = old_slr + added;

          sprintf(msgbuf, "WRG_EVENT TIP ch=%d slr=%.2f added=%.2f total=%.2f delta=%.2f new_bl=%.2f",
            c, old_slr, added, new_total, delta, reading);
          Output(msgbuf);

          dfrwrg_sp->sensor[c].rain_slr = new_total;
          dfrwrg_sp->sensor[c].baseline_sample = reading;
          dfrwrg_sp->sensor[c].last_sample = reading;

        } else if (delta >= WRG_NOISE_FLOOR_G) { // aka greather than .05 g
            // Real weight increase
            sprintf(msgbuf, "WRG_EVENT INC ch=%d delta=%.2f reading=%.2f bl=%.02f", c, delta, reading, baseline);
            Output(msgbuf);
            dfrwrg_sp->sensor[c].last_sample = reading;
        }
      }
    }
  }
}

/* 
 *=======================================================================================================================
 * dfrwrg_setup() - DFRobot Gravity: HX711 Weight Sensors Init Data Structure Storage - called from setup()
 *=======================================================================================================================
 */
void dfrwrg_setup() { 
  // set up the bucket structure to capture sensors
  dfrwrg_sp = &dfrwrg_sensors;
  dfrwrg_sp->number_found = 0;
}   

/* 
 *=======================================================================================================================
 * dfrwrg_init() - initialize DFRobot Gravity: HX711 Weight Sensor
 *=======================================================================================================================
 */
void dfrwrg_init(DFRobot_HX711_I2C *&dfrwrg, CH_SENSOR *chs, int mux_channel) {  // *& = reference to a pointer
  Output ("  INITIALIZING");

  if (dfrwrg == nullptr) {
    // We need to set the structure up once. We will reuse it on all the mux channels
    // We are using the same i2c address on all mux channels for this sensor
    // Output("DFRobot_GAS_I2C() Init");
    dfrwrg = new DFRobot_HX711_I2C(&Wire, chs->address);
  }

  if (!dfrwrg->begin()) { 
    Output ("  OFFLINE");
  }
  else {
    dfrwrg_sp->sensor[mux_channel].rain_slr = 0; // grams

    float reading = dfrwrg->readWeight(); // reads 12 times and returns the average.
    dfrwrg_sp->sensor[mux_channel].baseline_sample = reading;
    dfrwrg_sp->sensor[mux_channel].last_sample = reading;
    sprintf (msgbuf, "  BASELINE:%.2f g", reading);
    Output (msgbuf);

    dfrwrg_sp->sensor[mux_channel].calibration = dfrwrg->getCalibration();
    dfrwrg->setCalibration(dfrwrg_sp->sensor[mux_channel].calibration);

    sprintf (msgbuf, "  CALVAL:%.2f", dfrwrg_sp->sensor[mux_channel].calibration);
    Output (msgbuf);

    chs->state = ONLINE;
    Output ("  ONLINE");
    dfrwrg_sp->number_found++;
  }
}