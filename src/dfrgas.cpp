/*
 * ======================================================================================================================
 *  dfrgas.cpp - DFRobot Multi Gas Sensors Functions
 * ======================================================================================================================
 */
#include "include/output.h"
#include "include/analog.h"
#include "include/main.h"
#include "include/sensors.h"
#include "include/dfrgas.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
DFRobot_GAS_I2C *dfrgas = nullptr;

 /*
 * ======================================================================================================================
 *  DFRobot GAS Sensor
 * ======================================================================================================================
 */
DFRGAS_SENSORS dfrgas_sensors;
DFRGAS_SENSORS *dfrgas_sp;

/* 
 *=======================================================================================================================
 * dfrgas_TakeReading() -- 
 *=======================================================================================================================
 */
void dfrgas_TakeReading() {

  if (dfrgas_sp->number_found) {
    for (int c=0; c<MUX_CHANNELS; c++) {
      mc = &mux[c];
      if (mc->inuse) {
        // Loop through sensors on channel
        for (int s=0; s<MAX_CHANNEL_SENSORS; s++) {
          chs = &mc->sensor[s];
          if ((chs->type == dfrg) && (chs->state == ONLINE)) {

            mux_channel_set(c); // Set mux channel

            dfrgas_sp->sensor[c].bucket[dfrgas_sp->bucket_idx] = dfrgas->readGasConcentrationPPM();
          }
        }
      }
    }
    // All sensors share the same index counter
    ++dfrgas_sp->bucket_idx;
    dfrgas_sp->bucket_idx = (dfrgas_sp->bucket_idx) % DFRGAS_READINGS; // Advance bucket index for next reading
  }
}

/* 
 *=======================================================================================================================
 * dfrgas_fill() -- 
 *=======================================================================================================================
 */
void dfrgas_fill() {
  if (dfrgas_sp->number_found) {
    for (int i=0; i<DFRGAS_READINGS; i++) {
      dfrgas_TakeReading();
      delay (990);
    }
  }
}

/* 
 *=======================================================================================================================
 * dfrgas_readAvg() -- returns the average for the gas sensor on this mux channel
 *=======================================================================================================================
 */
float dfrgas_readAvg(int c) {
  float avg = 0.0F;

  for (int i=0; i<DFRGAS_READINGS; i++) {
    avg += dfrgas_sp->sensor[c].bucket[i];
  }
  avg = avg / DFRGAS_READINGS;

  return (avg);
}

/* 
 *=======================================================================================================================
 * dfrgas_isValidGasType() - 
 *=======================================================================================================================
 */
bool dfrgas_isValidGasType(const char* type) {
    if (type == nullptr) return false;

    return strcmp(type, "O2")  == 0 ||
           strcmp(type, "CO")  == 0 ||
           strcmp(type, "H2S") == 0 ||
           strcmp(type, "NO2") == 0 ||
           strcmp(type, "O3")  == 0 ||
           strcmp(type, "CL2") == 0 ||
           strcmp(type, "NH3") == 0 ||
           strcmp(type, "H2")  == 0 ||
           strcmp(type, "HCL") == 0 ||
           strcmp(type, "SO2") == 0 ||
           strcmp(type, "HF")  == 0 ||
           strcmp(type, "PH3") == 0;
}

/* 
 *=======================================================================================================================
 * dfrgas_setup() - DFRobot Multi Gas Sensors Init Data Structure Storage - called from setup()
 *=======================================================================================================================
 */
void dfrgas_setup() { 
  // set up the bucket structure to capture sensors
  dfrgas_sp = &dfrgas_sensors;
  dfrgas_sp->bucket_idx = 0;
  dfrgas_sp->number_found = 0;
}   

/* 
 *=======================================================================================================================
 * dfrgas_init() - initialize DFRobot Multi Gas Sensors
 *=======================================================================================================================
 */
void dfrgas_init(DFRobot_GAS_I2C *&dfrgas, CH_SENSOR *chs, int mux_channel) {  // *& = reference to a pointer
  Output ("  INITIALIZING");

  if (dfrgas == nullptr) {
    // We need to set the structure up once. We will reuse it on all the mux channels
    // We are using the same i2c address on all mux channels for this sensor
    // Output("DFRobot_GAS_I2C() Init");
    dfrgas = new DFRobot_GAS_I2C(&Wire, chs->address);
     
    // Output("DFRobot_GAS_I2C() Init After");
  }

  if (!dfrgas->begin()) { 
    Output ("  OFFLINE");
  }
  else {
    String type = dfrgas->queryGasType(); // O2, CO, H2S, NO2, O3, CL2, NH3, H2, HCL, SO2, HF, PH

    if (dfrgas_isValidGasType(type.c_str())) {
      // Save gas sensor type 
      type.toLowerCase(); // Save sensor type in lowercase. Better for tag names when reporting obs.
      strncpy (dfrgas_sp->sensor[mux_channel].type, type.c_str(), sizeof(dfrgas_sp->sensor[mux_channel].type) - 1);
  
      // Mode of obtaining data: Sensor auto-uploads data every 1 second 
      Output ("  AQUIRE MODE:INITIATIVE");
      dfrgas->changeAcquireMode(dfrgas->INITIATIVE);

      // Turn on temperature compensation
      Output ("  TEMP COMPENSATION:ON");
      dfrgas->setTempCompensation(dfrgas->ON);

      sprintf (msgbuf, "  TYPE:%s", dfrgas_sp->sensor[mux_channel].type);
      Output (msgbuf);

      float gas_reading = dfrgas->readGasConcentrationPPM();
      sprintf (msgbuf, "  SAMPLE:%.2f PPM", gas_reading);
      Output (msgbuf);

      chs->state = ONLINE;
      Output ("  ONLINE");
      dfrgas_sp->number_found++;
    }
    else {
      sprintf (msgbuf, "  OFFLINE - GAS TYPE[%s] UNKNOWN", type.c_str());
      Output (msgbuf);
    } 
  }
}