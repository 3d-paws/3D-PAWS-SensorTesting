/*
 * ======================================================================================================================
 *  sensirion_sen66.cpp - Sensirion sen66 Sensor Functions
 * ======================================================================================================================
 */
#include "include/output.h"
#include "include/analog.h"
#include "include/main.h"
#include "include/sensors.h"
#include "include/sensirion_sen66.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

 /*
 * ======================================================================================================================
 *  Sensirion Sen66 Sensor
 * ======================================================================================================================
 */
SensirionI2cSen66 *sen66 = nullptr;

SEN66_SENSORS sen66_sensors;
SEN66_SENSORS *sen66_sp;

/* 
 *=======================================================================================================================
 * sen66_TakeReading() -- 
 *=======================================================================================================================
 */
void sen66_TakeReading() {

  if (sen66_sp->number_found) {

    float Pm1p0 = 0.0f;
    float Pm2p5 = 0.0f;
    float Pm4p0 = 0.0f;
    float Pm10p0 = 0.0f;
    float h = 0.0f;
    float t = 0.0f;
    float voc = 0.0f;
    float nox = 0.0f;
    uint16_t co2 = 0;

    for (int c=0; c<MUX_CHANNELS; c++) {
      mc = &mux[c];
      if (mc->inuse) {
        // Loop through sensors on channel
        for (int s=0; s<MAX_CHANNEL_SENSORS; s++) {
          chs = &mc->sensor[s];
          if ((chs->type == s66) && (chs->state == ONLINE)) {

            mux_channel_set(c); // Set mux channel

            if (sen66->readMeasuredValues(Pm1p0, Pm2p5, Pm4p0, Pm10p0, h, t, voc, nox, co2) == NO_ERROR) {
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].Pm1p0 = Pm1p0;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].Pm2p5 = Pm2p5;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].Pm4p0 = Pm4p0;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].Pm10p0 = Pm10p0;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].h = h;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].t = t;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].voc = voc;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].nox = nox;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].co2 = co2;
            }
            else {
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].Pm1p0 = 0.0f;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].Pm2p5 = 0.0f;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].Pm4p0 = 0.0f;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].Pm10p0 = 0.0f;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].h = 0.0f;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].t = 0.0f;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].voc = 0.0f;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].nox = 0.0f;
              sen66_sp->sensor[c].bucket[sen66_sp->bucket_idx].co2 = 0;              
            }
          }
        }
      }
    }
    // All sensors share the same index counter
    ++sen66_sp->bucket_idx;
    sen66_sp->bucket_idx = (sen66_sp->bucket_idx) % SEN66_READINGS; // Advance bucket index for next reading
  }
}

/* 
 *=======================================================================================================================
 * dfrgas_readAvg() -- returns the average for the gas sensor on this mux channel
 *=======================================================================================================================
 */
void sen66_readAvg(int c, SEN66_BUCKET &avg) {
  avg = {};
  uint32_t co2sum = 0; // avg.co2 is a uint16_t using 32bit here to prevent overflow.

  for (int i=0; i<SEN66_READINGS; i++) {
    avg.Pm1p0 += sen66_sp->sensor[c].bucket[i].Pm1p0;
    avg.Pm2p5 += sen66_sp->sensor[c].bucket[i].Pm2p5;
    avg.Pm4p0 += sen66_sp->sensor[c].bucket[i].Pm4p0;;
    avg.Pm10p0 += sen66_sp->sensor[c].bucket[i].Pm10p0;
    avg.h += sen66_sp->sensor[c].bucket[i].h;
    avg.t += sen66_sp->sensor[c].bucket[i].t;
    avg.voc += sen66_sp->sensor[c].bucket[i].voc;
    avg.nox += sen66_sp->sensor[c].bucket[i].nox;
    co2sum += sen66_sp->sensor[c].bucket[i].co2;
  }
  avg.Pm1p0 = avg.Pm1p0 / SEN66_READINGS;
  avg.Pm2p5 = avg.Pm2p5 / SEN66_READINGS;
  avg.Pm4p0 = avg.Pm4p0 / SEN66_READINGS;
  avg.Pm10p0 = avg.Pm10p0 / SEN66_READINGS;
  avg.h = avg.h / SEN66_READINGS;
  avg.t = avg.t / SEN66_READINGS;
  avg.voc = avg.voc / SEN66_READINGS;
  avg.nox = avg.nox / SEN66_READINGS;
  avg.co2 = co2sum / SEN66_READINGS;
}

/* 
 *=======================================================================================================================
 * sen66_setup() - Sensirion Sen66 Sensors Init Data Structure Storage - called from setup()
 *=======================================================================================================================
 */
void sen66_setup() { 
  // set up the bucket structure to capture sensors
  sen66_sp = &sen66_sensors;
  sen66_sp->bucket_idx = 0;
  sen66_sp->number_found = 0; 
}   

/* 
 *=======================================================================================================================
 * sen66_init() - Sensirion Sen66 Sensors
 *=======================================================================================================================
 */
void sen66_init(SensirionI2cSen66 *&sen66, CH_SENSOR *chs, int mux_channel) {  // *& = reference to a pointer
  int8_t buffer[64] = {0};
  uint8_t firmwareMajor;
  uint8_t firmwareMinor;

  Output ("  INITIALIZING");

  if (sen66 == nullptr) {
    // We need to set the structure up once. We will reuse it on all the mux channels
    // We are using the same i2c address on all mux channels for this sensor
    // Output("sen66 Init");
    sen66 = new SensirionI2cSen66();
    sen66->begin(Wire, chs->address);

    // Output("sen66 Init After");
  }

  if (sen66->deviceReset() != NO_ERROR) {
    Output ("  OFFLINE:Reset Failed");
    return;
  }

  delay(1200);

  if (sen66->setSensorAltitude(site_elevation) != NO_ERROR) {
    Output ("  OFFLINE:Set Altitude Failed");
    return;     
  }
  else {
    sprintf (msgbuf, "  ALT:%dm", site_elevation);
    Output (msgbuf);
  }

  if (sen66->startContinuousMeasurement() != NO_ERROR) {
    Output ("  OFFLINE:Set Continuous Measurement Failed");
    return;
  }
  else {
    sprintf (msgbuf, "  CONT.MEASURE:SET");
    Output (msgbuf);
  }

  if (sen66->getProductName(buffer, 64) != NO_ERROR) {
    Output ("  OFFLINE:Get Product Name Failed");
    return; 
  }
  else {
    sprintf (msgbuf, "  PRODUCT:%s", buffer);
    Output (msgbuf);
  }

  if (sen66->getProductType(buffer, 64) != NO_ERROR) {
    Output ("  OFFLINE:Get Product Type Failed");
    return; 
  }
  else {
    sprintf (msgbuf, "  TYPE:%s", buffer);
    Output (msgbuf);
  }  

  if (sen66->getSerialNumber(buffer, 32) != NO_ERROR) {
    Output ("  OFFLINE:Get Product SN Failed");
    return; 
  }
  else {
    sprintf (msgbuf, "  SN:%s", buffer);
    Output (msgbuf);
  }

  if (sen66->getVersion(firmwareMajor, firmwareMinor) != NO_ERROR) {
    Output ("  OFFLINE:Get Version Failed");
    return; 
  }
  else {
    sprintf (msgbuf, "  FW:%d.%d", firmwareMajor,firmwareMinor);
    Output (msgbuf);
  }
  Output ("DELAY 10s For SEN66 To Become Ready");
  delay(10000);

  float Pm1p0 = 0.0f;
  float Pm2p5 = 0.0f;
  float Pm4p0 = 0.0f;
  float Pm10p0 = 0.0f;
  float h = 0.0f;
  float t = 0.0f;
  float voc = 0.0f;
  float nox = 0.0f;
  uint16_t co2 = 0;

  if (sen66->readMeasuredValues(Pm1p0, Pm2p5, Pm4p0,Pm10p0, h, t, voc, nox, co2) != NO_ERROR) {
    Output ("  OFFLINE:Get Readings Failed");
    return; 
  }
  else {
    sprintf (msgbuf, "  Pm1p0:%.2f",  Pm1p0); Output (msgbuf);
    sprintf (msgbuf, "  Pm2p5:%.2f",  Pm2p5); Output (msgbuf);
    sprintf (msgbuf, "  Pm4p0:%.2f",  Pm4p0); Output (msgbuf);
    sprintf (msgbuf, "  Pm10p0:%.2f", Pm10p0); Output (msgbuf);

    sprintf (msgbuf, "  Humidity:%.2f", h); Output (msgbuf);
    sprintf (msgbuf, "  Temp:%.2f",     t); Output (msgbuf);
    sprintf (msgbuf, "  VOC:%.2f",    voc); Output (msgbuf);
    sprintf (msgbuf, "  NOX:%.2f",    nox); Output (msgbuf);
    sprintf (msgbuf, "  CO2:%d",      co2); Output (msgbuf);
  }

  chs->state = ONLINE;
  Output ("  ONLINE");
  sen66_sp->number_found++;
}