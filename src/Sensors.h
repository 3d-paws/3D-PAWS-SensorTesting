/*
 * ======================================================================================================================
 *  Sensors.h
 * ======================================================================================================================
 */

#define MUX_CHANNELS 8
#define MAX_CHANNEL_SENSORS 10
#define MUX_ADDR 0x70
#define HIH8000_ADDRESS   0x27

typedef enum {
  UNKN, bmp, bme, b38, b39, htu, sht, mcp, hdc, lps, hih, tlw, tsm 
} SENSOR_TYPE;
const char *sensor_type[] = {"UNKN", "bmp", "bme", "b38", "b39", "htu", "sht", "mcp", "hdc", "lps", "hih", "tlw", "tsm"};

typedef enum { 
  OFFLINE,
  ONLINE
} SENSOR_STATE;

const char *sensor_state[] = {"OFFLINE", "ONLINE"};

typedef struct {
  SENSOR_STATE  state;
  SENSOR_TYPE   type;
  byte          id;
  byte          address;
} CH_SENSOR;

typedef struct {
  bool            inuse;                // Set to true when an observation is stored here         
  CH_SENSOR       sensor[MAX_CHANNEL_SENSORS];
} MULTIPLEXER_STR;

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
 * htu  0x40 only this address
 *
 * sht  0x44 Default
 * sht  0x45 with jumper  
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
 * hih  0x27   only this address
 * 
 * Tinovi Leaf Wetness
 * tlw  0x61   only this address
 * 
 * Tinovi Soil Moisture
 * tsm  0x63   only this address
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
  mux[1].sensor[0].type = b39;
  mux[1].sensor[0].id = 1;
  mux[1].sensor[0].address = 0x77;

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
  mux[3].sensor[0].type = lps;
  mux[3].sensor[0].id = 1;
  mux[3].sensor[0].address = 0x5D;

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

Adafruit_BME280 bme1;
Adafruit_BME280 bme2;
Adafruit_BME280 bme3;

Adafruit_BMP3XX b381;
Adafruit_BMP3XX b382;
Adafruit_BMP3XX b383;

Adafruit_BMP3XX b391;
Adafruit_BMP3XX b392;
Adafruit_BMP3XX b393;

Adafruit_HTU21DF htu1;
Adafruit_HTU21DF htu2;
Adafruit_HTU21DF htu3;

Adafruit_SHT31 sht1;
Adafruit_SHT31 sht2;
Adafruit_SHT31 sht3;

Adafruit_MCP9808 mcp1;
Adafruit_MCP9808 mcp2;
Adafruit_MCP9808 mcp3;

Adafruit_HDC302x hdc1;
Adafruit_HDC302x hdc2;
Adafruit_HDC302x hdc3;

Adafruit_LPS35HW lps1;
Adafruit_LPS35HW lps2;
Adafruit_LPS35HW lps3;

LeafSens tlw1;
LeafSens tlw2;
LeafSens tlw3;

SVCS3 tsm1;
SVCS3 tsm2;
SVCS3 tsm3;

/* 
 *=======================================================================================================================
 * mux_channel_set() - set mux channel
 *=======================================================================================================================
 */
void mux_channel_set(uint8_t i) {
  if (i > 7) return;
 
  sprintf (Buffer32Bytes, "MUX:CHANNEL:%d SET", i);
  Output (Buffer32Bytes);
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
          sprintf (Buffer32Bytes, "  CH:%d S:%d,%s%d,0x%02x", 
          c, s, sensor_type[chs->type], chs->id, chs->address);
          Output (Buffer32Bytes);

          // Initialize sensor mapping to appropriate substantiation
          switch (chs->type) {

            // BMP
            case bmp :
              switch (chs->id) {
                case 1 : 
                  if (!bmp1.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = bmp1.readPressure();
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  if (!bmp2.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = bmp2.readPressure();
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  if (!bmp3.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = bmp3.readPressure();
                    Output ("    ONLINE");
                  } 
                  break;
                default :
                  Output ("    Invalid Sensor ID");
                  break;
              }
              break;

            // BME
            case bme :
              switch (chs->id) {
                case 1 : 
                  if (!bme1.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = bme1.readPressure();
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  if (!bme2.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = bme2.readPressure();
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  if (!bme3.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = bme3.readPressure();
                    Output ("    ONLINE");
                  } 
                  break;

                default :
                  Output ("    Invalid Sensor ID");
                  break;
              }
              break;

            // B38
            case b38 :
              switch (chs->id) {
                case 1 : 
                  if (!b381.begin_I2C(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = b381.readPressure();
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  if (!b382.begin_I2C(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = b382.readPressure();
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  if (!b383.begin_I2C(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = b383.readPressure();
                    Output ("    ONLINE");
                  } 
                  break;

                default :
                  Output ("    Invalid Sensor ID");
                  break;
              }
              break;

            // B39
            case b39 :
              switch (chs->id) {
                case 1 : 
                  if (!b391.begin_I2C(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = b391.readPressure();
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  if (!b392.begin_I2C(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = b392.readPressure();
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  if (!b393.begin_I2C(chs->address)) { 
                    Output ("OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    float p = b393.readPressure();
                    Output ("    ONLINE");
                  } 
                  break;

                default :
                  Output ("    nvalid Sensor ID");
                  break;
              }
              break;

            // HTU
            case htu :
              switch (chs->id) {
                case 1 : 
                  if (!htu1.begin()) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  if (!htu2.begin()) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  if (!htu1.begin()) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                default :
                  Output ("    Invalid Sensor ID");
                  break;
              } 
              break;

            // SHT
            case sht :
              switch (chs->id) {
                case 1 :
                  sht1 = Adafruit_SHT31();
                  if (!sht1.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  sht2 = Adafruit_SHT31();
                  if (!sht2.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  sht3 = Adafruit_SHT31();
                  if (!sht3.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                default :
                  Output ("    Invalid Sensor ID");
                  break;
              }
              break;

            // MCP
            case mcp :
              switch (chs->id) {
                case 1 :
                  mcp1 = Adafruit_MCP9808();
                  if (!mcp1.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  mcp2 = Adafruit_MCP9808();
                  if (!mcp2.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  mcp3 = Adafruit_MCP9808();
                  if (!mcp3.begin(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                default :
                  Output ("    Invalid Sensor ID");
                  break;
              }
              break;

            // HDC
            case hdc :
              switch (chs->id) {
                case 1 :
                  hdc1 = Adafruit_HDC302x();
                  if (!hdc1.begin(chs->address, &Wire)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    double t,h;
                    hdc1.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0);
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  hdc2 = Adafruit_HDC302x();
                  if (!hdc2.begin(chs->address, &Wire)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    double t,h;
                    hdc2.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0);
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  hdc3 = Adafruit_HDC302x();
                  if (!hdc3.begin(chs->address, &Wire)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    double t,h;
                    hdc3.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0);
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                default :
                  Output ("    Invalid Sensor ID");
                  break;
              }
              break;

            // LPS
            case lps :
              switch (chs->id) {
                case 1 :
                  lps1 = Adafruit_LPS35HW();
                  if (!lps1.begin_I2C(chs->address, &Wire)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  lps2 = Adafruit_LPS35HW();
                  if (!lps2.begin_I2C(chs->address, &Wire)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  lps3 = Adafruit_LPS35HW();
                  if (!lps3.begin_I2C(chs->address, &Wire)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                default :
                  Output ("    Invalid Sensor ID");
                  break;
              }
              break;

            // HIH
            case hih :
              switch (chs->id) {
                case 1 :
                  if (!I2C_Device_Exist(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  if (!I2C_Device_Exist(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  if (!I2C_Device_Exist(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                default :
                  Output ("    Invalid Sensor ID");
                  break;
              }
              break;

            // Tinovi Leaf Wetness
            case tlw :
              switch (chs->id) {
                case 1 :
                  if (!I2C_Device_Exist(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    tlw1.init(chs->address);
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  if (!I2C_Device_Exist(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    tlw2.init(chs->address);
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  if (!I2C_Device_Exist(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    tlw3.init(chs->address);
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                default :
                  Output ("    Invalid Sensor ID");
                  break;
              }
              break;

            // Tinovi Soil Moisture
            case tsm :
              switch (chs->id) {
                case 1 :
                  if (!I2C_Device_Exist(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    tsm1.init(chs->address);
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  }
                  break;

                case 2 :
                  if (!I2C_Device_Exist(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    tsm2.init(chs->address);
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                case 3 :
                  if (!I2C_Device_Exist(chs->address)) { 
                    Output ("    OFFLINE");
                  }
                  else {
                    tsm3.init(chs->address);
                    chs->state = ONLINE;
                    Output ("    ONLINE");
                  } 
                  break;

                default :
                  Output ("    Invalid Sensor ID");
                  break;
              }
              break;

            // Default  
            default :
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
 * hih8_getTempHumid() - Get Temp and Humidity
 *   Call example:  status = hih8_getTempHumid(&t, &h);
 *=======================================================================================================================
 */
bool hih8_getTempHumid(float *t, float *h) {

  // Set Error Values as our Defaults
  *h = QC_ERR_RH;
  *t = QC_ERR_T;

  uint16_t humidityBuffer    = 0;
  uint16_t temperatureBuffer = 0;
  
  Wire.begin();
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