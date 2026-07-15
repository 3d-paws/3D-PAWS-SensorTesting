/*
 * ======================================================================================================================
 *  dfrgas.h - DFRobot Multi Gas Sensors
 * 
 * SEE https://github.com/DFRobot/DFRobot_MultiGasSensor
 * 
 * Supported Gas Sensors: o2, co, h2s, no2, o3, cl2, nh3, h2, hcl, so2, hf, ph
 * 
 * Reported tags name examples dfrg-co-1 where 1 is the id you specifed in mux_sensor_config()
 * ======================================================================================================================
 */
#include <DFRobot_MultiGasSensor.h>
#include "include/sensors.h"

// CO Sensor Range is 0-1000 PPM

#define DFRGAS_READINGS   60       // One minute of 1s Samples
#define DFRGAS_MUX_CHANNELS 8

typedef struct {
  float bucket[DFRGAS_READINGS];
  char type[10];
} DFRGAS_SENSOR;

typedef struct {
  DFRGAS_SENSOR sensor[DFRGAS_MUX_CHANNELS];
  int bucket_idx;
  int number_found;
} DFRGAS_SENSORS;

// Extern variables
extern DFRGAS_SENSORS dfrgas_sensors;
extern DFRGAS_SENSORS *dfrgas_sp;
extern DFRobot_GAS_I2C *dfrgas;    // All the Sensors mush have the same i2c address, so we are supporting 1 per mux channel


// Function prototype
void dfrgas_setup();
void dfrgas_init(DFRobot_GAS_I2C *&dfrgas, CH_SENSOR *chs, int mux_channel);
void dfrgas_TakeReading();
void dfrgas_fill();
float dfrgas_readAvg(int c);