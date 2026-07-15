/*
 * ======================================================================================================================
 * sensirion_sen66.h -
 * 
 * The SEN66 reports a set of environmental measurements, including particulate matter as PM1.0, PM2.5, PM4, and PM10,
 * plus temperature, humidity, CO2, VOC index, and NOx index. The particulate readings are mass concentrations 
 * in μg/m3, where the PM number is the largest particle size included in that bin: PM1 counts particles 
 * up to 1.0 μm, PM2.5 up to 2.5 μm, PM4 up to 4.0 μm, and PM10 up to 10 μm
 * 
 * Time basis for the reading
 * The sensor is designed for continuous measurements, with a minimum sampling interval of 1 second in continuous mode. 
 * In practice, the particulate number you read is a smoothed, continuously updated estimate rather than an instantaneous 
 * snapshot of a single second. Sensirion’s documentation describes PM4 and PM10 as calculated from the distribution 
 * profile of all measured particles, which implies the output reflects an internal accumulation/estimation process over 
 * recent measurements rather than a raw one-sample value.
 * 
 * Treat the PM outputs as a regularly updated air-quality estimate in μg/m3
 * 
 * 
 * The altitude setting is there because the SEN66’s gas-related measurements, especially CO2, are affected by 
 * air pressure and altitude. Setting altitude lets the sensor or its firmware compensate for the lower 
 * atmospheric pressure at your location so the readings stay more accurate.
 * 
 * In practice, altitude is an easier proxy for ambient pressure when you don’t want to measure pressure directly. 
 * The driver also exposes a separate ambient pressure setting, which tells you the compensation is part of the 
 * sensor’s calibration and not just a convenience option. For an installation at a fixed site, you normally 
 * set the real elevation once and leave it alone.
 * 
 *   uint16_t altitude_m = 1600;   // your site elevation in meters
 *   sen66.setSensorAltitude(altitude_m);
 *   sen66.startContinuousMeasurement();
 * 
 * If you don’t call setSensorAltitude(), the SEN66 uses the default altitude of 0 meters above sea level. 
 * In other words, it assumes sea-level pressure unless you tell it otherwise. That mainly affects the 
 * gas-related compensation path, especially CO2 behavior, not the particulate measurements. 
 * So the sensor will still run and produce readings normally; they’ll just be compensated as if it were 
 * installed at sea level
 * 
 * Altitude error mainly shows up as a bias in the CO2 reading, not random noise. A good rule of thumb 
 * from CO2 sensor guidance is about 3% error per 1,000 feet of altitude if the sensor is 
 * not compensated correctly.
 * 1,000 ft elevation error: about 3% CO2 error.
 * 3,000 ft: about 9% error.
 * 5,000 ft: about 15% error
 * 
 * Sensor:
 * https://www.digikey.com/en/product-highlight/s/sensirion/environmental-sensor-node-sen6x
 * https://blog.adafruit.com/2025/05/08/eye-on-npi-sensirion-sen66-environmental-sensor-node-eyeonnpi-digikey-digikey-sensirion-adafruit/?__cf_chl_f_tk=4V8omfv0Z8YDJHyrZiKB0c4rqStQ8TmXBRCuf4lLHao-1782850522-1.0.1.1-83zXxmNkrltBZaGNNRiX1CKZgz7gKmdEYzvnyKAVQZk
 * 
 * Code:
 * https://github.com/Sensirion/arduino-i2c-sen66
 * Use Arduino IDE to fetch Sensirion_I2C_SEN66 and Sensirion_Core
 * 
 * Breakout: 
 * Adafruit SEN6x Breakout for Sensirion SEN66 - STEMMA QT / Qwiic
 * https://www.adafruit.com/product/6331
 * https://learn.adafruit.com/adafruit-sen6x-breakout
 * The adapter breakout for the SEN6x board has a JST GH-compatible connector in the middle which you can plug a 
 * 6-pin GH cable directly between your SEN6x and this adapter. In the middle is a clean 3.3V regulator and 
 * level shifter, so you can use this module whether you're using 3.3V or 5V power and logic. 
 * Then you can use STEMMA QT cables to plug into your I2C buss.
 * ======================================================================================================================
 */
#include <SensirionI2cSen66.h>
#include "include/sensors.h"

#define SEN66_READINGS     60       // One minute of 1s Samples
#define SEN66_MUX_CHANNELS 8

typedef struct {
  float Pm1p0;
  float Pm2p5;
  float Pm4p0;
  float Pm10p0;
  float h;
  float t;
  float voc;
  float nox;
  uint16_t co2;
} SEN66_BUCKET;

typedef struct {
  SEN66_BUCKET bucket[SEN66_READINGS];
} SEN66_SENSOR;

typedef struct {
  SEN66_SENSOR sensor[SEN66_MUX_CHANNELS];
  int bucket_idx;
  int number_found;
} SEN66_SENSORS;

// Extern variables
extern SEN66_SENSORS sen66_sensors;
extern SEN66_SENSORS *sen66_sp;

extern SensirionI2cSen66 *sen66;

// Function prototype
void sen66_setup();
void sen66_init(SensirionI2cSen66 *&sen66, CH_SENSOR *chs, int mux_channel);
void sen66_TakeReading();
void sen66_readAvg(int c, SEN66_BUCKET &avg);
