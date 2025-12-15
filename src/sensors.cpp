/*
 * ======================================================================================================================
 *  sensors.cpp - I2C based sensors
 * ======================================================================================================================
 */
#include "include/qc.h"
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/sensors.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
const char *sensor_state[] = {"OFFLINE", "ONLINE"};
const char *sensor_type[] = {"UNKN", "bmp", "bme", "b38", "b39", "b58", "htu", "sht3", "sht4", "mcp", "hdc", "lps", "si", "ltr", "hih", "tlw", "tsm", "dfrlx"};
MULTIPLEXER_STR mux[MUX_CHANNELS];
MULTIPLEXER_STR *mc;
CH_SENSOR *chs;

/* 
 *=======================================================================================================================
 * mux_sensor_config() - define sensor configuration
 * 
 * bmp  0x77 default
 * bmp  0x76 with jumper
 * 
 * bme  0x77 default
 * bme  0x76 with jumper
 * 
 * b38  0x77 default
 * b38  0x76 with jumper
 * 
 * b39  0x77 default
 * b39  0x76 with jumper
 * 
 * b58  0x47 default           ChipID BMP580 = 0x50, BMP581 = 0x51
 * b58  0x46 with jumper
 * 
 * htu  0x40 only this address
 *
 * sht3 0x44 Default
 * sht3 0x45 with jumper  
 * 
 * sht4 0x44 Default
 * 
 * dfrl 0x23 Default
 * 
 *           AD2 AD1 AD0
 * mcp  0x18   0   0   0  Default Address
 * mcp  0x19   0   0   1
 * mcp  0x1A   0   1   0
 * mcp  0x1B   0   1   1
 * mcp  0x1C   1   0   0
 * mcp  0x1D   1   0   1
 * mcp  0x1E   1   1   0
 * mcp  0x1F   1   1   1
 * 
 *             A1   A0
 * hdc  0x44    0    0  Default
 * hdc  0x45    0    1 
 * hdc  0x46    1    0
 * hdc  0x47    1    1
 * 
 * lps  0x5D   default
 * lps  0x5C   with jumper
 *
 * Si1145 UV/IR/Visible Light Sensor
 * si   0x60   only this address
 * 
 * LTR390 UV Sensor
 * ltr  0x53   only this address
 * 
 * HIH8000
 * hih  0x27   only this address
 * 
 * Tinovi Leaf Wetness
 * tlw  0x61   only this address
 * 
 * Tinovi Soil Moisture
 * tsm  0x63   only this address
 * 
 * DFRobot SEN0562 Ambient Light Sensor 1-65535lx / BH1750
 * dfrl 0x23   only this address
 *=======================================================================================================================
 */
void mux_sensor_config() {
  // Mux Channel 0
  mux[0].inuse = true;
  // mcp1
  mux[0].sensor[0].type = mcp;
  mux[0].sensor[0].id = 1;
  mux[0].sensor[0].address = 0x18;
  
  // Mux Channel 1
  mux[1].inuse = true;
  // b391
  mux[1].sensor[0].type = b58;
  mux[1].sensor[0].id = 1;
  mux[1].sensor[0].address = 0x47;
  // SHT41
  mux[1].sensor[1].type = sht4;
  mux[1].sensor[1].id = 1;
  mux[1].sensor[1].address = 0x44;
  // LTR1
  mux[1].sensor[2].type = ltr;
  mux[1].sensor[2].id = 1;
  mux[1].sensor[2].address = 0x53;

  // Mux Channel 2
  mux[2].inuse = true;

  mux[2].sensor[0].type = hdc;
  mux[2].sensor[0].id = 1;
  mux[2].sensor[0].address = 0x46;

  mux[2].sensor[1].type = hdc;
  mux[2].sensor[1].id = 2;
  mux[2].sensor[1].address = 0x47;

  // Mux Channel 3
  mux[3].inuse = true;

  mux[3].sensor[0].type = si;
  mux[3].sensor[0].id = 1;
  mux[3].sensor[0].address = 0x60;
  // DFRL
  mux[3].sensor[1].type = dfrl;
  mux[3].sensor[1].id = 1;
  mux[3].sensor[1].address = 0x23;

  // Mux Channel 4
  mux[4].inuse = true;
  mux[4].sensor[0].type = tlw;
  mux[4].sensor[0].id = 1;
  mux[4].sensor[0].address = 0x61;

  mux[4].sensor[1].type = tsm;
  mux[4].sensor[1].id = 1;
  mux[4].sensor[1].address = 0x63;

  // Mux Channel 5
  // Mux Channel 6
  // Mux Channel 7
}

/* 
 *=======================================================================================================================
 * mux_sensor_config() - Substantiate sensors
 *=======================================================================================================================
 */
Adafruit_BMP280 bmp1;
Adafruit_BMP280 bmp2;
Adafruit_BMP280 bmp3;
Adafruit_BMP280 bmp4;

Adafruit_BME280 bme1;
Adafruit_BME280 bme2;
Adafruit_BME280 bme3;
Adafruit_BME280 bme4;

Adafruit_BMP3XX b381;
Adafruit_BMP3XX b382;
Adafruit_BMP3XX b383;
Adafruit_BMP3XX b384;

Adafruit_BMP3XX b391;
Adafruit_BMP3XX b392;
Adafruit_BMP3XX b393;
Adafruit_BMP3XX b394;

Adafruit_BMP5xx b581;
Adafruit_BMP5xx b582;
Adafruit_BMP5xx b583;
Adafruit_BMP5xx b584;

Adafruit_HTU21DF htu1;
Adafruit_HTU21DF htu2;
Adafruit_HTU21DF htu3;
Adafruit_HTU21DF htu4;

Adafruit_SHT31 sht31;
Adafruit_SHT31 sht32;
Adafruit_SHT31 sht33;
Adafruit_SHT31 sht34;
Adafruit_SHT31 sht35;
Adafruit_SHT31 sht36;
Adafruit_SHT31 sht37;
Adafruit_SHT31 sht38;

Adafruit_SHT4x sht41;
Adafruit_SHT4x sht42;
Adafruit_SHT4x sht43;
Adafruit_SHT4x sht44;
Adafruit_SHT4x sht45;
Adafruit_SHT4x sht46;
Adafruit_SHT4x sht47;
Adafruit_SHT4x sht48;

Adafruit_MCP9808 mcp1;
Adafruit_MCP9808 mcp2;
Adafruit_MCP9808 mcp3;
Adafruit_MCP9808 mcp4;

Adafruit_HDC302x hdc1;
Adafruit_HDC302x hdc2;
Adafruit_HDC302x hdc3;
Adafruit_HDC302x hdc4;

Adafruit_LPS35HW lps1;
Adafruit_LPS35HW lps2;
Adafruit_LPS35HW lps3;
Adafruit_LPS35HW lps4;

Adafruit_SI1145 si1;
Adafruit_SI1145 si2;
Adafruit_SI1145 si3;
Adafruit_SI1145 si4;

Adafruit_LTR390 ltr1;
Adafruit_LTR390 ltr2;
Adafruit_LTR390 ltr3;
Adafruit_LTR390 ltr4;

LeafSens tlw1;
LeafSens tlw2;
LeafSens tlw3;
LeafSens tlw4;

SVCS3 tsm1;
SVCS3 tsm2;
SVCS3 tsm3;
SVCS3 tsm4;

BH1750 dfrl1;
BH1750 dfrl2;
BH1750 dfrl3;
BH1750 dfrl4;

/* 
 *=======================================================================================================================
 * mux_channel_set() - set mux channel
 *=======================================================================================================================
 */
void mux_channel_set(uint8_t i) {
  if (i > 7) return;
/*
  sprintf (Buffer32Bytes, "MUX:CHANNEL:%d SET", i);
  Output (Buffer32Bytes);
*/
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

/* 
 *=======================================================================================================================
 * mux_sensor_list() - list sensor configuration
 *=======================================================================================================================
 */
void mux_sensor_list() {
  Output("MUX:SENSOR:LIST");

  for (int c=0; c<MUX_CHANNELS; c++) {
    mc = &mux[c];
    if (mc->inuse == false) {
      // sprintf (Buffer32Bytes, "CH:%d NOT IN USE", c);
      // Output (Buffer32Bytes);
    }
    else { 
      for (int s=0; s<MAX_CHANNEL_SENSORS; s++) {
        chs = &mc->sensor[s];

        if (chs->type == UNKN) {
          // sprintf (Buffer32Bytes, "CH:%d S:%02d NOT IN USE", c, s);
          // Output (Buffer32Bytes);
        }
        else {
          sprintf (Buffer32Bytes, "CH:%d S:%d,%s%d,0x%02x,%s", 
          c, s, sensor_type[chs->type], chs->id, chs->address,  sensor_state[chs->state]);
          Output (Buffer32Bytes);
        }
      }
    }
  }
}

/* 
 *=======================================================================================================================
 * bmp_init() - initialize BME280
 *=======================================================================================================================
 */
void bmp_init(Adafruit_BMP280 &bmp, CH_SENSOR *chs) {
  if (!bmp.begin(chs->address)) { 
    Output ("OFFLINE");
  }
  else {
    chs->state = ONLINE;
    bmp.readPressure();
    Output ("ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * bme_init() - initialize BME280
 *=======================================================================================================================
 */
void bme_init(Adafruit_BME280 &bme, CH_SENSOR *chs) {
  if (!bme.begin(chs->address)) { 
    Output ("OFFLINE");
  }
  else {
    chs->state = ONLINE;
    bme.readPressure();
    Output ("ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * b3x_init() - initialize BMP3XX
 *=======================================================================================================================
 */
void b3x_init(Adafruit_BMP3XX &bmp, CH_SENSOR *chs) {
  if (!bmp.begin_I2C(chs->address)) { 
    Output ("OFFLINE");
  }
  else {
    chs->state = ONLINE;
    bmp.readPressure();
    Output ("ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * bm58x_detail() - display bm58x details using the unified sensor API
 *=======================================================================================================================
 */
void bm58x_detail(Adafruit_BMP5xx &bmp) {
  // Get separate sensor objects for temperature and pressure
  Adafruit_Sensor *bmp_temp = NULL;
  Adafruit_Sensor *bmp_pressure = NULL;
  sensor_t sensor;

  bmp_temp = bmp.getTemperatureSensor();
  bmp_temp->getSensor(&sensor);

  Output ("BM58X Temperature Sensor");
  sprintf (Buffer32Bytes, " Sensor Name: %s",   sensor.name);
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Sensor Type: %02lx", sensor.type);
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Driver Ver: %02lx", sensor.version);
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Unique ID: %02lx", sensor.sensor_id);
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Min Value: %d.%02d C", (int)sensor.min_value, (int)(sensor.min_value*100)%100); 
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Max Value: %d.%02d C", (int)sensor.max_value, (int)(sensor.max_value*100)%100);
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Resol: %d.%02d C", (int)sensor.resolution, (int)(sensor.resolution*100)%100);
  Output(Buffer32Bytes);

  bmp_pressure = bmp.getPressureSensor();
  bmp_pressure->getSensor(&sensor);
  Output ("BM58X Pressure Sensor");
  sprintf (Buffer32Bytes, " Sensor Name: %s",   sensor.name);
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Sensor Type: %02lx", sensor.type);
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Driver Ver: %02lx", sensor.version);
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Unique ID: %02lx", sensor.sensor_id);
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Min Value: %d.%02d hPa", (int)sensor.min_value, (int)(sensor.min_value*100)%100); 
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Max Value: %d.%02d hPa", (int)sensor.max_value, (int)(sensor.max_value*100)%100);
  Output(Buffer32Bytes);
  sprintf (Buffer32Bytes, " Resol: %d.%02d hPa", (int)sensor.resolution, (int)(sensor.resolution*100)%100);
  Output(Buffer32Bytes);
}

/* 
 *=======================================================================================================================
 * bm58x_init() - initialize BMP5xx
 *=======================================================================================================================
 */
void bm58x_init(Adafruit_BMP5xx &bmp, CH_SENSOR *chs) {
  if (!bmp.begin(chs->address, &Wire)) { 
    Output ("OFFLINE");
  }
  else {
    chs->state = ONLINE;

    // Configure sensor for optimal performance
    bmp.setTemperatureOversampling(BMP5XX_OVERSAMPLING_2X);
    bmp.setPressureOversampling(BMP5XX_OVERSAMPLING_16X);
    bmp.setIIRFilterCoeff(BMP5XX_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP5XX_ODR_50_HZ);
    bmp.setPowerMode(BMP5XX_POWERMODE_NORMAL);

    bmp.readPressure();
    Output ("ONLINE");
    bm58x_detail(bmp);
  }
}

/* 
 *=======================================================================================================================
 * htu_init() - initialize HTU21DF
 *=======================================================================================================================
 */
void htu_init(Adafruit_HTU21DF &htu) {
  if (!htu.begin()) { 
    Output ("OFFLINE");
  }
  else {
    chs->state = ONLINE;
    Output ("ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * sht3_init() - initialize SHT31
 *=======================================================================================================================
 */
void sht3_init(Adafruit_SHT31 &sht3, CH_SENSOR *chs) {
  sht3 = Adafruit_SHT31();
  if (!sht3.begin(chs->address)) { 
    Output ("OFFLINE");
  }
  else {
    chs->state = ONLINE;
    Output ("ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * sht4_detail() - display sht4x details
 *=======================================================================================================================
 */
void sht4_detail(Adafruit_SHT4x &sht4) {


  Output ("SHT4x Information");
  sprintf (Buffer32Bytes, " Serial Number: %lX", sht4.readSerial());
  Output(Buffer32Bytes);

  switch (sht4.getPrecision()) {
     case SHT4X_HIGH_PRECISION:  // default
       Output(" High precision");
       break;
     case SHT4X_MED_PRECISION: 
       Output(" Med precision");
       break;
     case SHT4X_LOW_PRECISION: 
       Output(" Low precision");
       break;
  }

  switch (sht4.getHeater()) {
     case SHT4X_NO_HEATER:   // default
       Output(" No heater");
       break;
     case SHT4X_HIGH_HEATER_1S: 
       Output(" High heat for 1 second");
       break;
     case SHT4X_HIGH_HEATER_100MS: 
       Output( "High heat for 0.1 second");
       break;
     case SHT4X_MED_HEATER_1S: 
       Output(" Medium heat for 1 second");
       break;
     case SHT4X_MED_HEATER_100MS: 
       Output(" Medium heat for 0.1 second");
       break;
     case SHT4X_LOW_HEATER_1S: 
       Output(" Low heat for 1 second");
       break;
     case SHT4X_LOW_HEATER_100MS: 
       Output(" Low heat for 0.1 second");
       break;
  }
}

/* 
 *=======================================================================================================================
 * sht4_init() - initialize SHT4x
 *=======================================================================================================================
 */
void sht4_init(Adafruit_SHT4x &sht4, CH_SENSOR *chs) {
  sht4 = Adafruit_SHT4x();
  if (!sht4.begin()) { 
    Output ("OFFLINE");
  }
  else {
    chs->state = ONLINE;
    Output ("ONLINE");

    // You can have 3 different precisions, higher precision takes longer
    sht4.setPrecision(SHT4X_HIGH_PRECISION);

    // You can have 6 different heater settings
    // higher heat and longer times uses more power
    // and reads will take longer too!
    sht4.setHeater(SHT4X_NO_HEATER);

    sht4_detail(sht4);
  }
}

/* 
 *=======================================================================================================================
 * mcp_init() - initialize MCP9808
 *=======================================================================================================================
 */
void mcp_init(Adafruit_MCP9808 &mcp, CH_SENSOR *chs) {
  mcp = Adafruit_MCP9808();
  if (!mcp.begin(chs->address)) { 
    Output ("OFFLINE");
  }
  else {
    chs->state = ONLINE;
    Output ("ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * hdc_init() - initialize HDC302x
 *=======================================================================================================================
 */
void hdc_init(Adafruit_HDC302x &hdc, CH_SENSOR *chs) {
  hdc = Adafruit_HDC302x();
  if (!hdc.begin(chs->address, &Wire)) { 
    Output ("OFFLINE");
  }
  else {
    double t,h;
    hdc.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0);
    chs->state = ONLINE;
    Output ("ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * lps_init() - initialize LPS35HW
 *=======================================================================================================================
 */
void lps_init(Adafruit_LPS35HW &lps, CH_SENSOR *chs) {
  lps = Adafruit_LPS35HW();
  if (!lps.begin_I2C(chs->address, &Wire)) { 
    Output ("OFFLINE");
  }
  else {
    chs->state = ONLINE;
    Output ("ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * si_init() - initialize SI1145
 *=======================================================================================================================
 */
void si_init(Adafruit_SI1145 &si, CH_SENSOR *chs) {
  if (!si.begin(chs->address)) { 
    Output ("OFFLINE");
  }
  else {
    chs->state = ONLINE;
    Output ("ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * ltr_detail() - display LTR390 details
 *=======================================================================================================================
 */
void ltr_detail(Adafruit_LTR390 &ltr) {

  Output ("LTR Information");

  if (ltr.getMode() == LTR390_MODE_ALS) {
    Output(" In ALS mode"); // default
  } else {
    Output(" In UVS mode");
  }

  switch (ltr.getGain()) {
    case LTR390_GAIN_1:  Output (" GAIN: 1"); break;
    case LTR390_GAIN_3:  Output (" GAIN: 3"); break; // Default
    case LTR390_GAIN_6:  Output (" GAIN: 6"); break;
    case LTR390_GAIN_9:  Output (" GAIN: 9"); break;
    case LTR390_GAIN_18: Output (" GAIN: 18"); break;
  }

  switch (ltr.getResolution()) {
    case LTR390_RESOLUTION_13BIT: Output (" Resolution: 13"); break;
    case LTR390_RESOLUTION_16BIT: Output (" Resolution: 16"); break; 
    case LTR390_RESOLUTION_17BIT: Output (" Resolution: 17"); break;
    case LTR390_RESOLUTION_18BIT: Output (" Resolution: 18"); break; // Default
    case LTR390_RESOLUTION_19BIT: Output (" Resolution: 19"); break;
    case LTR390_RESOLUTION_20BIT: Output (" Resolution: 20"); break;
  }
}

/* 
 *=======================================================================================================================
 * ltr_init() - initialize LTR390
 *=======================================================================================================================
 */
void ltr_init(Adafruit_LTR390 &ltr, CH_SENSOR *chs) {
  ltr = Adafruit_LTR390();

  if (!ltr.begin()) { 
    Output ("OFFLINE");
  }
  else {
    chs->state = ONLINE;
    Output ("ONLINE");

    // LTR390_MODE_UVS and LTR390_MODE_ALS are two operational modes for the LTR390 sensor, each targeting a different 
    // type of light measurement. LTR390_MODE_UVS configures the sensor to measure ultraviolet (UV) light, specifically 
    // in the UVA range (around 300–350 nm), which is useful for UV index calculations and monitoring UV exposure. 
    // LTR390_MODE_ALS, on the other hand, sets the sensor to measure ambient light (ALS), which is in the visible 
    // spectrum (typically 500–600 nm), similar to how the human eye perceives light.
    ltr.setMode(LTR390_MODE_UVS); // default ALS mode

    ltr.setGain(LTR390_GAIN_3); // default 3 - Higher gain values increase sensitivity but may introduce more noise.

    ltr.setResolution(LTR390_RESOLUTION_16BIT); // default 16 Bits - balances measurement precision and speed.

    // ltr.setThresholds(100, 1000); // disabled - defines the lower and upper thresholds for the interrupt. 
                                     // When the UV reading drops below 100 or exceeds 1000, the interrupt will trigger.

    ltr.configInterrupt(false, LTR390_MODE_UVS); // disabled - true would enable the interrupt

    ltr_detail(ltr);
  }
}

/* 
 *=======================================================================================================================
 * hih_init() - initialize HIH8000
 *=======================================================================================================================
 */
void hih_init(CH_SENSOR *chs) {
  if (!I2C_Device_Exist(chs->address)) { 
    Output ("  OFFLINE");
  }
  else {
    chs->state = ONLINE;
    Output ("  ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * tlw_init() - initialize Tinovi Leaf Wetness
 *=======================================================================================================================
 */
void tlw_init(LeafSens &tlw, CH_SENSOR *chs) {
  if (!I2C_Device_Exist(chs->address)) { 
    Output ("  OFFLINE");
  }
  else {
    tlw.init(chs->address);
    chs->state = ONLINE;
    Output ("  ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * tsm_init() - initialize Tinovi Soil Moisture
 *=======================================================================================================================
 */
void tsm_init(SVCS3 &tsm, CH_SENSOR *chs) {
  if (!I2C_Device_Exist(chs->address)) { 
    Output ("  OFFLINE");
  }
  else {
    tsm.init(chs->address);
    chs->state = ONLINE;
    Output ("  ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * dfrl_init() - initialize DFRobot SEN0562 Ambient Light Sensor 1-65535lx - BH1750
 *=======================================================================================================================
 */
void dfrl_init(BH1750 &dfrl, CH_SENSOR *chs) {
  if (!dfrl.begin()) { 
    Output ("  OFFLINE");
  }
  else {
    chs->state = ONLINE;
    Output ("  ONLINE");
  }
}

/* 
 *=======================================================================================================================
 * mux_sensor_initialize() - 
 *=======================================================================================================================
 */
void mux_sensor_initialize() {
  Output("MUX:SENSOR:INIT");
  for (int c=0; c<MUX_CHANNELS; c++) {
    mc = &mux[c];
    if (mc->inuse) {
      mux_channel_set(c); // Set mux channel

      for (int s=0; s<MAX_CHANNEL_SENSORS; s++) {
        chs = &mc->sensor[s];

        if (chs->type != UNKN) {
          sprintf (Buffer32Bytes, "CH:%d S:%d,%s%d,0x%02x", 
          c, s, sensor_type[chs->type], chs->id, chs->address);
          Output (Buffer32Bytes);

          // Initialize sensor mapping to appropriate substantiation
          switch (chs->type) {
            case bmp : // BMP280
              switch (chs->id) {
                case 1 : bmp_init(bmp1, chs); break;
                case 2 : bmp_init(bmp2, chs); break;
                case 3 : bmp_init(bmp3, chs); break;
                case 4 : bmp_init(bmp4, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case bme : // BME280
              switch (chs->id) {
                case 1 : bme_init(bme1, chs); break;
                case 2 : bme_init(bme2, chs); break;
                case 3 : bme_init(bme3, chs); break;
                case 4 : bme_init(bme4, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case b38 : // BMP388
              switch (chs->id) {
                case 1 : b3x_init(b381, chs); break;
                case 2 : b3x_init(b382, chs); break;
                case 3 : b3x_init(b383, chs); break;
                case 4 : b3x_init(b384, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case b39 : // BMP390
              switch (chs->id) {
                case 1 : b3x_init(b391, chs); break;
                case 2 : b3x_init(b392, chs); break;
                case 3 : b3x_init(b393, chs); break;
                case 4 : b3x_init(b394, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case b58 : // BMP581
              switch (chs->id) {
                case 1 : bm58x_init(b581, chs); break;
                case 2 : bm58x_init(b582, chs); break;
                case 3 : bm58x_init(b583, chs); break;
                case 4 : bm58x_init(b584, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case htu : // HTU21DF
              switch (chs->id) {
                case 1 : htu_init(htu1); break;
                case 2 : htu_init(htu2); break;
                case 3 : htu_init(htu3); break;
                case 4 : htu_init(htu4); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case sht3 : // SHT31
              switch (chs->id) {
                case 1 : sht3_init(sht31, chs); break;
                case 2 : sht3_init(sht32, chs); break;
                case 3 : sht3_init(sht33, chs); break;
                case 4 : sht3_init(sht34, chs); break;
                case 5 : sht3_init(sht35, chs); break;
                case 6 : sht3_init(sht36, chs); break;
                case 7 : sht3_init(sht37, chs); break;
                case 8 : sht3_init(sht38, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

              case sht4 : // SHT4x - SHT45
              switch (chs->id) {
                case 1 : sht4_init(sht41, chs); break;
                case 2 : sht4_init(sht42, chs); break;
                case 3 : sht4_init(sht43, chs); break;
                case 4 : sht4_init(sht44, chs); break;
                case 5 : sht4_init(sht45, chs); break;
                case 6 : sht4_init(sht46, chs); break;
                case 7 : sht4_init(sht47, chs); break;
                case 8 : sht4_init(sht48, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case mcp : // MCP9808
              switch (chs->id) {
                case 1 : mcp_init(mcp1, chs); break;
                case 2 : mcp_init(mcp2, chs); break;
                case 3 : mcp_init(mcp3, chs); break;
                case 4 : mcp_init(mcp4, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case hdc : // HDC302x
              switch (chs->id) {
                case 1 : hdc_init(hdc1, chs); break;
                case 2 : hdc_init(hdc2, chs); break;
                case 3 : hdc_init(hdc3, chs); break;
                case 4 : hdc_init(hdc4, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case lps : // LPS35HW
              switch (chs->id) {
                case 1 : lps_init(lps1, chs); break;
                case 2 : lps_init(lps2, chs); break;
                case 3 : lps_init(lps3, chs); break;
                case 4 : lps_init(lps4, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case si : // Si1145 UV/IR/Visible Light Sensor
              switch (chs->id) {
                case 1 : si_init(si1, chs); break;
                case 2 : si_init(si2, chs); break;
                case 3 : si_init(si3, chs); break;
                case 4 : si_init(si4, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case ltr : // LTR390
              switch (chs->id) {
                case 1 : ltr_init(ltr1, chs); break;
                case 2 : ltr_init(ltr2, chs); break;
                case 3 : ltr_init(ltr3, chs); break;
                case 4 : ltr_init(ltr4, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case hih : // HIH8000
              switch (chs->id) {
                case 1 : hih_init(chs); break;
                case 2 : hih_init(chs); break;
                case 3 : hih_init(chs); break;
                case 4 : hih_init(chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case tlw : // Tinovi Leaf Wetness
              switch (chs->id) {
                case 1 : tlw_init(tlw1, chs); break;
                case 2 : tlw_init(tlw2, chs); break;
                case 3 : tlw_init(tlw3, chs); break;
                case 4 : tlw_init(tlw4, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case tsm : // Tinovi Soil Moisture
              switch (chs->id) {
                case 1 : tsm_init(tsm1, chs); break;
                case 2 : tsm_init(tsm2, chs); break;
                case 3 : tsm_init(tsm3, chs); break;
                case 4 : tsm_init(tsm4, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            case dfrl : // DFRobot SEN0562 Ambient Light Sensor 1-65535lx BH1750
              switch (chs->id) {
                case 1 : dfrl_init(dfrl1, chs); break;
                case 2 : dfrl_init(dfrl2, chs); break;
                case 3 : dfrl_init(dfrl3, chs); break;
                case 4 : dfrl_init(dfrl4, chs); break;
                default : Output ("  Invalid Sensor ID"); break;
              }
              break;

            default : // Default  
              break;
          }
        }
      }
    }
  }
}

/* 
 *=======================================================================================================================
 * mux_initialize() - 
 *=======================================================================================================================
 */
void mux_initialize() {
  Output("MUX:INIT");

  for (int c=0; c<MUX_CHANNELS; c++) {
    mc = &mux[c];
    mc->inuse = false;
    for (int s=0; s<MAX_CHANNEL_SENSORS; s++) {
      chs = &mc->sensor[s];
      chs->state = OFFLINE;
      chs->type = UNKN;
      chs->address = 0x00;
      chs->id = 0;
    }
  }

  mux_sensor_config();
  mux_sensor_list();
  mux_sensor_initialize();

  Output("MUX:INIT EXIT");
}

/* 
 *=======================================================================================================================
 * hih_getTempHumid() - Get Temp and Humidity
 *   Call example:  status = hih8_getTempHumid(&t, &h);
 *=======================================================================================================================
 */
bool hih8_getTempHumid(float *t, float *h) {

  // Set Error Values as our Defaults
  *h = QC_ERR_RH;
  *t = QC_ERR_T;

  uint16_t humidityBuffer    = 0;
  uint16_t temperatureBuffer = 0;
  
  Wire.beginTransmission(HIH8000_ADDRESS);

  Wire.write(0x00); // set the register location for read request

  delayMicroseconds(200); // give some time for sensor to process request

  if (Wire.requestFrom(HIH8000_ADDRESS, 4) == 4) {

    // Get raw humidity data
    humidityBuffer = Wire.read();
    humidityBuffer <<= 8;
    humidityBuffer |= Wire.read();
    humidityBuffer &= 0x3FFF;   // 14bit value, get rid of the upper 2 status bits

    // Get raw temperature data
    temperatureBuffer = Wire.read();
    temperatureBuffer <<= 8;
    temperatureBuffer |= Wire.read();
    temperatureBuffer >>= 2;  // Remove the last two "Do Not Care" bits (shift left is same as divide by 4)

    Wire.endTransmission();

    *h = humidityBuffer * 6.10e-3;
    *t = temperatureBuffer * 1.007e-2 - 40.0;

    // QC Check
    *h = (isnan(*h) || (*h < QC_MIN_RH) || (*h >QC_MAX_RH)) ? QC_ERR_RH : *h;
    *t = (isnan(*t) || (*t < QC_MIN_T)  || (*t >QC_MAX_T))  ? QC_ERR_T  : *t;
    return (true);
  }
  else {
    Wire.endTransmission();
    return(false);
  }
}