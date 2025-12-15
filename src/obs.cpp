/*
 * ======================================================================================================================
 *  OBS.h - Observation Handeling
 * ======================================================================================================================
 */
#include <Particle.h>
#include <SdFat.h>

#include "include/qc.h"
#include "include/ps.h"
#include "include/sensors.h"
#include "include/sdcard.h"
#include "include/output.h"
#include "include/support.h"
#include "include/time.h"
#include "include/main.h"
#include "include/analog.h"
#include "include/wind.h"
#include "include/obs.h"

/*
 * ======================================================================================================================
 * Particle_Publish() - Publish to Particle what is in msgbuf
 * ======================================================================================================================
 */
bool Particle_Publish(char *EventName) {
  // Calling Particle.publish() when the cloud connection has been turned off will not publish an event. 
  // This is indicated by the return success code of false. If the cloud connection is turned on and 
  // trying to connect to the cloud unsuccessfully, Particle.publish() may block for up to 20 seconds 
  // (normal conditions) to 10 minutes (unusual conditions). Checking Particle.connected() 
  // before calling Particle.publish() can help prevent this.
  // if (Cellular.ready() && Particle.connected()) {
  if (Particle.connected()) {
    if (Particle.publish(EventName, msgbuf, WITH_ACK)) { // PRIVATE flag is always used even when not specified
      // Currently, a device can publish at rate of about 1 event/sec, with bursts of up to 4 allowed in 1 second. 
      delay (1000);
      return(true);
    }
  }
  else {
    Output ("Particle:NotReady");
  }
  return(false);
}

/*
 * ======================================================================================================================
 * OBS_Do() - Collect Observations, Build message, Send to logging site
 * ======================================================================================================================
 */
void OBS_Do() {
  float t;
  float p;
  float h;
  double dt = -999.9;
  double dh = -999.9;
  float e25;
  float ec;
  float vwc;
  float w; // wet
  float si_vis = 0.0;
  float si_ir = 0.0;
  float si_uv = 0.0;
  float lux = 0.0;
  uint32_t ltr_uv;

  float BatteryPoC = 0.0; // Battery Percent of Charge


  // Safty Check for Vaild Time
  if (!Time.isValid()) {
    Output ("OBS_Do: Time NV");
    return;
  }

  // Output("OBS_Do:");

  // stc_timestamp();  //System Clock
  // Output(timestamp);

  rtc_timestamp();  // Use RTC Clock, Global Variable "DateTime now;" is updated and timestamp string set

  memset(msgbuf, 0, sizeof(msgbuf));
  JSONBufferWriter writer(msgbuf, sizeof(msgbuf)-1);

  writer.beginObject();
  writer.name("at").value(timestamp);
  writer.name("epoch").value(now.unixtime());

#if PLATFORM_ID == PLATFORM_ARGON
  int BatteryState = 0;
  WiFiSignal sig = WiFi.RSSI();
  byte cfr = 0;
#else
  int BatteryState = System.batteryState();
  CellularSignal sig = Cellular.RSSI();
  byte cfr = pmic.getFault(); // Get Battery Charger Failt Register
  if (BatteryState>0 && BatteryState<6) {
    BatteryPoC = System.batteryCharge();
  }
#endif
  writer.name("bcs").value(BatteryState); // Battery Charging State
  writer.name("bpc").value(BatteryPoC,2); // Battery Percent Charge
  writer.name("cfr").value(cfr); // Battery Charger Fault Register

  for (int c=0; c<MUX_CHANNELS; c++) {
    mc = &mux[c];
    if (mc->inuse) {
      mux_channel_set(c); // Set mux channel

      for (int s=0; s<MAX_CHANNEL_SENSORS; s++) {
        chs = &mc->sensor[s];

        if (chs->type != UNKN) {
          /*
          sprintf (Buffer32Bytes, "CH:%d S:%d,%s%d,0x%02x,%s", 
            c, s, sensor_type[chs->type], chs->id, chs->address,
            sensor_state[chs->state]);
          Output (Buffer32Bytes);
          */

          if (chs->state == OFFLINE) {
            continue;  // Skip reading this sensor
          }

          // Initialize sensor mapping to appropriate substantiation
          switch (chs->type) {

            case bmp : // BMP280
              switch (chs->id) {
                case 1 :
                  t = bmp1.readTemperature();
                  p = bmp1.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("bmp1t").value(t, 2);
                  writer.name("bmp1p").value(p, 4);
                  break;

                case 2 :
                  t = bmp2.readTemperature();
                  p = bmp2.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("bmp2t").value(t, 2);
                  writer.name("bmp2p").value(p, 4);
                  break;

                case 3 :
                  t = bmp3.readTemperature();
                  p = bmp3.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("bmp3t").value(t, 2);
                  writer.name("bmp3p").value(p, 4);
                  break;

                case 4 :
                  t = bmp4.readTemperature();
                  p = bmp4.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("bmp4t").value(t, 2);
                  writer.name("bmp4p").value(p, 4);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case bme : // BME280
              switch (chs->id) {
                case 1 : 
                  t = bme1.readTemperature();
                  p = bme1.readPressure()/100.0F;
                  h = bme1.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("bme1t").value(t, 2);
                  writer.name("bme1p").value(p, 4);
                  writer.name("bme1h").value(h, 2);
                  break;

                case 2 :
                  t = bme2.readTemperature();
                  p = bme2.readPressure()/100.0F;
                  h = bme2.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("bme2t").value(t, 2);
                  writer.name("bme2p").value(p, 4);
                  writer.name("bme2h").value(h, 2);
                  break;

                case 3 :
                  t = bme3.readTemperature();
                  p = bme3.readPressure()/100.0F;
                  h = bme3.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("bme3t").value(t, 2);
                  writer.name("bme3p").value(p, 4);
                  writer.name("bme3h").value(h, 2);
                  break;

                case 4 :
                  t = bme4.readTemperature();
                  p = bme4.readPressure()/100.0F;
                  h = bme4.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("bme4t").value(t, 2);
                  writer.name("bme4p").value(p, 4);
                  writer.name("bme4h").value(h, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case b38 : // BMP388
              switch (chs->id) {
                case 1 : 
                  t = b381.readTemperature();
                  p = b381.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b381t").value(t, 2);
                  writer.name("b381p").value(p, 4);
                  break;

                case 2 :
                  t = b382.readTemperature();
                  p = b382.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b382t").value(t, 2);
                  writer.name("b382p").value(p, 4);
                  break;

                case 3 :
                  t = b383.readTemperature();
                  p = b383.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b383t").value(t, 2);
                  writer.name("b383p").value(p, 4);
                  break;

                case 4 :
                  t = b384.readTemperature();
                  p = b384.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b384t").value(t, 2);
                  writer.name("b384p").value(p, 4);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case b39 : // BMP390
              switch (chs->id) {
                case 1 : 
                  t = b391.readTemperature();
                  p = b391.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b391t").value(t, 2);
                  writer.name("b391p").value(p, 4);
                  break;

                case 2 :
                  t = b392.readTemperature();
                  p = b392.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b392t").value(t, 2);
                  writer.name("b392p").value(p, 4);
                  break;

                case 3 :
                  t = b393.readTemperature();
                  p = b393.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b393t").value(t, 2);
                  writer.name("b393p").value(p, 4);
                  break;

                case 4 :
                  t = b394.readTemperature();
                  p = b394.readPressure()/100.0F;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b394t").value(t, 2);
                  writer.name("b394p").value(p, 4);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case b58 : // BMP581
              switch (chs->id) {
                case 1 : 
                  t = b581.readTemperature();
                  p = b581.readPressure();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b581t").value(t, 2);
                  writer.name("b581p").value(p, 4);
                  break;

                case 2 :
                  t = b582.readTemperature();
                  p = b582.readPressure();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b582t").value(t, 2);
                  writer.name("b582p").value(p, 4);
                  break;

                case 3 :
                  t = b583.readTemperature();
                  p = b583.readPressure();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b583t").value(t, 2);
                  writer.name("b583p").value(p, 4);
                  break;

                case 4 :
                  t = b584.readTemperature();
                  p = b584.readPressure();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("b584t").value(t, 2);
                  writer.name("b584p").value(p, 4);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case htu : // HTU21DF
              switch (chs->id) {
                case 1 : 
                  t = htu1.readTemperature();
                  h = htu1.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("htu1t").value(t, 2);
                  writer.name("htu1h").value(h, 2);
                  break;

                case 2 :
                  t = htu2.readTemperature();
                  h = htu2.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("htu2t").value(t, 2);
                  writer.name("htu2h").value(h, 2);
                  break;

                case 3 :
                  t = htu3.readTemperature();
                  h = htu3.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("htu3t").value(t, 2);
                  writer.name("htu3h").value(h, 2);
                  break;

                case 4 :
                  t = htu4.readTemperature();
                  h = htu4.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("htu4t").value(t, 2);
                  writer.name("htu4h").value(h, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              } 
              break;

            case sht3 : // SHT31
              switch (chs->id) {
                case 1 :
                  t = sht31.readTemperature();
                  h = sht31.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht31t").value(t, 2);
                  writer.name("sht31h").value(h, 2);
                  break;

                case 2 :
                  t = sht32.readTemperature();
                  h = sht32.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht32t").value(t, 2);
                  writer.name("sht32h").value(h, 2);
                  break;

                case 3 :
                  t = sht33.readTemperature();
                  h = sht33.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht33t").value(t, 2);
                  writer.name("sht33h").value(h, 2);
                  break;

                case 4 :
                  t = sht34.readTemperature();
                  h = sht34.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht34t").value(t, 2);
                  writer.name("sht34h").value(h, 2);
                  break;

                case 5 :
                  t = sht35.readTemperature();
                  h = sht35.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht35t").value(t, 2);
                  writer.name("sht35h").value(h, 2);
                  break;

                case 6 :
                  t = sht36.readTemperature();
                  h = sht36.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht36t").value(t, 2);
                  writer.name("sht36h").value(h, 2);
                  break;

                case 7 :
                  t = sht37.readTemperature();
                  h = sht37.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht37t").value(t, 2);
                  writer.name("sht37h").value(h, 2);
                  break;

                case 8 :
                  t = sht38.readTemperature();
                  h = sht38.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht38t").value(t, 2);
                  writer.name("sht38h").value(h, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case sht4 : // SHT4x - SHT45
              switch (chs->id) {
                sensors_event_t humidity, temp;
                case 1 :
                  sht41.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
                  t = temp.temperature;
                  h = humidity.relative_humidity;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht41t").value(t, 2);
                  writer.name("sht41h").value(h, 2);
                  break;

                case 2 :
                  sht42.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
                  t = temp.temperature;
                  h = humidity.relative_humidity;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht42t").value(t, 2);
                  writer.name("sht42h").value(h, 2);
                  break;

                case 3 :
                  sht43.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
                  t = temp.temperature;
                  h = humidity.relative_humidity;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht43t").value(t, 2);
                  writer.name("sht43h").value(h, 2);
                  break;

                case 4 :
                  sht44.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
                  t = temp.temperature;
                  h = humidity.relative_humidity;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht44t").value(t, 2);
                  writer.name("sht44h").value(h, 2);
                  break;

                case 5 :
                  sht45.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
                  t = temp.temperature;
                  h = humidity.relative_humidity;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht45t").value(t, 2);
                  writer.name("sht45h").value(h, 2);
                  break;

                case 6 :
                  sht46.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
                  t = temp.temperature;
                  h = humidity.relative_humidity;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht46t").value(t, 2);
                  writer.name("sht46h").value(h, 2);
                  break;

                case 7 :
                  sht47.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
                  t = temp.temperature;
                  h = humidity.relative_humidity;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht47t").value(t, 2);
                  writer.name("sht47h").value(h, 2);
                  break;

                case 8 :
                  sht48.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
                  t = temp.temperature;
                  h = humidity.relative_humidity;
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht48t").value(t, 2);
                  writer.name("sht48h").value(h, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case mcp : // MCP9808
              switch (chs->id) {
                case 1 :
                  t = mcp1.readTempC();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("mcp1t").value(t, 2);
                  break;

                case 2 :
                  t = mcp2.readTempC();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("mcp2t").value(t, 2);
                  break;

                case 3 :
                  t = mcp3.readTempC();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("mcp3t").value(t, 2);
                  break;

                case 4 :
                  t = mcp4.readTempC();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("mcp4t").value(t, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case hdc : // HDC302x
              switch (chs->id) {
                case 1 :
                  dt = -999.9;
                  dh = -999.9;
                  if (hdc1.readTemperatureHumidityOnDemand(dt, dh, TRIGGERMODE_LP0)) {
                    dt = (isnan(dt) || (dt < QC_MIN_T)  || (dt > QC_MAX_T))  ? QC_ERR_T  : dt;
                    dh = (isnan(dh) || (dh < QC_MIN_RH) || (dh > QC_MAX_RH)) ? QC_ERR_RH : dh;
                  }
                  writer.name("hdc1t").value(dt, 2);
                  writer.name("hdc1h").value(dh, 2);
                  break;

                case 2 :
                  dt = -999.9;
                  dh = -999.9;
                  if (hdc2.readTemperatureHumidityOnDemand(dt, dh, TRIGGERMODE_LP0)) {
                    dt = (isnan(dt) || (dt < QC_MIN_T)  || (dt > QC_MAX_T))  ? QC_ERR_T  : dt;
                    dh = (isnan(dh) || (dh < QC_MIN_RH) || (dh > QC_MAX_RH)) ? QC_ERR_RH : dh;
                  }
                  writer.name("hdc2t").value(dt, 2);
                  writer.name("hdc2h").value(dh, 2);
                  break;

                case 3 :
                  dt = -999.9;
                  dh = -999.9;
                  if (hdc3.readTemperatureHumidityOnDemand(dt, dh, TRIGGERMODE_LP0)) {
                    dt = (isnan(dt) || (dt < QC_MIN_T)  || (dt > QC_MAX_T))  ? QC_ERR_T  : dt;
                    dh = (isnan(dh) || (dh < QC_MIN_RH) || (dh > QC_MAX_RH)) ? QC_ERR_RH : dh;
                  }
                  writer.name("hdc3t").value(dt, 2);
                  writer.name("hdc3h").value(dh, 2);
                  break;

                case 4 :
                  dt = -999.9;
                  dh = -999.9;
                  if (hdc4.readTemperatureHumidityOnDemand(dt, dh, TRIGGERMODE_LP0)) {
                    dt = (isnan(dt) || (dt < QC_MIN_T)  || (dt > QC_MAX_T))  ? QC_ERR_T  : dt;
                    dh = (isnan(dh) || (dh < QC_MIN_RH) || (dh > QC_MAX_RH)) ? QC_ERR_RH : dh;
                  }
                  writer.name("hdc4t").value(dt, 2);
                  writer.name("hdc4h").value(dh, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case lps : // LPS35HW
              switch (chs->id) {
                case 1 :
                  t = lps1.readTemperature();
                  p = lps1.readPressure();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("lps1t").value(t, 2);
                  writer.name("lps1p").value(p, 4);
                  break;

                case 2 :
                  t = lps2.readTemperature();
                  p = lps2.readPressure();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("lps2t").value(t, 2);
                  writer.name("lps2p").value(p, 4);
                  break;

                case 3 :
                  t = lps3.readTemperature();
                  p = lps3.readPressure();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("lps3t").value(t, 2);
                  writer.name("lps3p").value(p, 4);
                  break;

                case 4 :
                  t = lps4.readTemperature();
                  p = lps4.readPressure();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
                  writer.name("lps4t").value(t, 2);
                  writer.name("lps4p").value(p, 4);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case si : // Si1145 UV/IR/Visible Light Sensor
              switch (chs->id) {
                case 1 :
                  si_vis = si1.readVisible();
                  si_ir = si1.readIR();
                  si_uv = si1.readUV()/100.0;
                  writer.name("si1v").value(si_vis, 2);
                  writer.name("si1i").value(si_ir, 2);
                  writer.name("si1u").value(si_uv, 2);
                  break;

                case 2 :
                  si_vis = si2.readVisible();
                  si_ir = si2.readIR();
                  si_uv = si2.readUV()/100.0;
                  writer.name("si2v").value(si_vis, 2);
                  writer.name("si2i").value(si_ir, 2);
                  writer.name("si2u").value(si_uv, 2);
                  break;

                case 3 :
                  si_vis = si3.readVisible();
                  si_ir = si3.readIR();
                  si_uv = si3.readUV()/100.0;
                  writer.name("si3v").value(si_vis, 2);
                  writer.name("si3i").value(si_ir, 2);
                  writer.name("si3u").value(si_uv, 2);
                  break;

                case 4 :
                  si_vis = si4.readVisible();
                  si_ir = si4.readIR();
                  si_uv = si4.readUV()/100.0;
                  writer.name("si4v").value(si_vis, 2);
                  writer.name("si4i").value(si_ir, 2);
                  writer.name("si4u").value(si_uv, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            // LTR390 UV Light Sensor
            case ltr :
              switch (chs->id) {
                case 1 :
                  ltr_uv = ltr1.readUVS();
                  writer.name("ltr1").value(ltr_uv);
                  break;

                case 2 :
                  ltr_uv = ltr4.readUVS();
                  writer.name("ltr4").value(ltr_uv);
                  break;

                case 3 :
                  ltr_uv = ltr4.readUVS();
                  writer.name("ltr4").value(ltr_uv);
                  break;

                case 4 :
                  ltr_uv = ltr4.readUVS();
                  writer.name("ltr4").value(ltr_uv);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case hih : // HIH8000
              switch (chs->id) {
                case 1 :
                  t = 0.0;
                  h = 0.0;

                  if (!hih8_getTempHumid(&t, &h)) {
                    t = -999.99;
                    h = 0.0;
                  }
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("hih1t").value(t, 2);
                  writer.name("hih1h").value(h, 2);
                  break;

                case 2 :
                  t = 0.0;
                  h = 0.0;

                  if (!hih8_getTempHumid(&t, &h)) {
                    t = -999.99;
                    h = 0.0;
                  }
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("hih2t").value(t, 2);
                  writer.name("hih2h").value(h, 2);
                  break;

                case 3 :
                  t = 0.0;
                  h = 0.0;

                  if (!hih8_getTempHumid(&t, &h)) {
                    t = -999.99;
                    h = 0.0;
                  }
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("hih3t").value(t, 2);
                  writer.name("hih3h").value(h, 2);
                  break;

                case 4 :
                  t = 0.0;
                  h = 0.0;

                  if (!hih8_getTempHumid(&t, &h)) {
                    t = -999.99;
                    h = 0.0;
                  }
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("hih4t").value(t, 2);
                  writer.name("hih4h").value(h, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case tlw :  // Tinovi Leaf Wetness
              switch (chs->id) {
                case 1 :
                  tlw1.newReading();
                  delay(100);
                  w = tlw1.getWet();
                  t = tlw1.getTemp();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("tlw1w").value(w, 2);
                  writer.name("tlw1t").value(t, 2);
                  break;

                case 2 :
                  tlw2.newReading();
                  delay(100);
                  w = tlw2.getWet();
                  t = tlw2.getTemp();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("tlw2w").value(w, 2);
                  writer.name("tlw2t").value(t, 2);
                  break;

                case 3 :
                  tlw3.newReading();
                  delay(100);
                  w = tlw3.getWet();
                  t = tlw3.getTemp();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("tlw3w").value(w, 2);
                  writer.name("tlw3t").value(t, 2);
                  break;

                case 4 :
                  tlw4.newReading();
                  delay(100);
                  w = tlw4.getWet();
                  t = tlw4.getTemp();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("tlw4w").value(w, 2);
                  writer.name("tlw4t").value(t, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case tsm : // Tinovi Soil Moisture
              switch (chs->id) {
                case 1 :
                  tsm1.newReading();
                  delay(100);
                  e25 = tsm1.getE25();
                  ec = tsm1.getEC();
                  vwc = tsm1.getVWC();
                  t = tsm1.getTemp();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("tlw1e25").value(e25, 2);
                  writer.name("tlw1ec").value(ec, 2);
                  writer.name("tlw1vwc").value(vwc, 2);
                  writer.name("tlw1t").value(t, 2);
                  break;

                case 2 :
                  tsm2.newReading();
                  delay(100);
                  e25 = tsm2.getE25();
                  ec = tsm2.getEC();
                  vwc = tsm2.getVWC();
                  t = tsm2.getTemp();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("tlw2e25").value(e25, 2);
                  writer.name("tlw2ec").value(ec, 2);
                  writer.name("tlw2vwc").value(vwc, 2);
                  writer.name("tlw2t").value(t, 2);
                  break;

                case 3 :
                  tsm3.newReading();
                  delay(100);
                  e25 = tsm3.getE25();
                  ec = tsm3.getEC();
                  vwc = tsm3.getVWC();
                  t = tsm3.getTemp();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("tlw3e25").value(e25, 2);
                  writer.name("tlw3ec").value(ec, 2);
                  writer.name("tlw3vwc").value(vwc, 2);
                  writer.name("tlw3t").value(t, 2);
                  break;

                case 4 :
                  tsm4.newReading();
                  delay(100);
                  e25 = tsm4.getE25();
                  ec = tsm4.getEC();
                  vwc = tsm4.getVWC();
                  t = tsm4.getTemp();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  writer.name("tlw4e25").value(e25, 2);
                  writer.name("tlw4ec").value(ec, 2);
                  writer.name("tlw4vwc").value(vwc, 2);
                  writer.name("tlw4t").value(t, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            case dfrl : // DFRobot SEN0562 Ambient Light Sensor 1-65535lx
              switch (chs->id) {
                case 1 :
                  lux = dfrl1.readLightLevel();
                  writer.name("dfrl1").value(lux, 2);
                  break;

                case 2 :
                  lux = dfrl2.readLightLevel();
                  writer.name("dfrl2").value(lux, 2);
                  break;

                case 3 :
                  lux = dfrl3.readLightLevel();
                  writer.name("dfrl3").value(lux, 2);
                  break;

                case 4 :
                  lux = dfrl4.readLightLevel();
                  writer.name("dfrl4").value(lux, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            // Default - Sensor tyoe not found
            default :
              break;
          }
        }
      }
    }
  }

  // Add Wind if whe have wind direction detected
  if (AS5600_exists) {
    Wind_GustUpdate(); // Update Gust and Gust Direction readings

    float ws = Wind_SpeedAverage();
    ws = (isnan(ws) || (ws < QC_MIN_WS) || (ws > QC_MAX_WS)) ? QC_ERR_WS : ws;

    int wd = Wind_DirectionVector();
    wd = (isnan(wd) || (wd < QC_MIN_WD) || (wd > QC_MAX_WD)) ? QC_ERR_WD : wd;

    float wg = Wind_Gust();
    wg = (isnan(wg) || (wg < QC_MIN_WS) || (wg > QC_MAX_WS)) ? QC_ERR_WS : wg;

    int wgd = Wind_GustDirection();
    wgd = (isnan(wgd) || (wgd < QC_MIN_WD) || (wgd > QC_MAX_WD)) ? QC_ERR_WD : wgd;

    writer.name("ws").value(ws,4);
    writer.name("wd").value(wd);
    writer.name("wg").value(wg,4);
    writer.name("wgd").value(wgd);
  }

  // Read and report Analog Pins
  for (int pin=0; pin<ANALOG_PIN_COUNT; pin++) {
    if (analog_pins[pin].inuse) {
      char tag[16];

      // Force a skip of A2 if we have detected wind direction sensor
      if ((pin==2) && AS5600_exists) {
        continue;
      }

      readAnalogPin(pin);
      sprintf (tag, "%sm", pinNames[pin]);
      writer.name(tag).value(analog_pins[pin].median);
      sprintf (tag, "%sr", pinNames[pin]);
      writer.name(tag).value(analog_pins[pin].raw);
      sprintf (tag, "%sa", pinNames[pin]);
      writer.name(tag).value(analog_pins[pin].average, 4);
    }
  }

  writer.endObject();

  // Log Observation to SD Card
  SD_LogObservation(msgbuf);
  Serial_write (msgbuf);

  if (OBS_Interval>=60) { // Only send to Particle if obs interval is grater than a minute
    Time_of_last_obs = Time.now();

    Output ("Publish(ST)");
    if (Particle_Publish((char *) "ST")) { 
      PostedResults = true;

      if (SD_exists) {
        sprintf (Buffer32Bytes, "Publish(OK)[%d]", strlen(msgbuf)+1);
        Output (Buffer32Bytes);
      }
      else {
        Output ("Publish(OK)-NO SD!!!");
      }
    }
    else {
      PostedResults = false;
      Output ("Publish(FAILED)");
    }
  }
}

