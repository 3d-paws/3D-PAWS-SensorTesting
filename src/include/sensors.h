/*
 * ======================================================================================================================
 *  sensors.h - I2C Sensor Definations
 * ======================================================================================================================
 */
#include <Adafruit_BMP280.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_BMP5xx.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_MCP9808.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_SHT4x.h>
#include <Adafruit_HDC302x.h>
#include <Adafruit_LPS35HW.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_LTR390.h>
#include <i2cArduino.h>
#include <LeafSens.h>
#include <BH1750.h> // DFRobot SEN0562 Ambient Light Sensor

#ifndef SENSORS_H
#define SENSORS_H

#define MUX_CHANNELS 8
#define MAX_CHANNEL_SENSORS 10
#define MUX_ADDR          0x70
#define HIH8000_ADDRESS   0x27
#define DFRL_ADDRESS      0x23

typedef enum {
  UNKN, bmp, bme, b38, b39, b58, htu, sht3, sht4, mcp, hdc, lps, si, ltr, hih, tlw, tsm, dfrl, dfrg, s66
} SENSOR_TYPE;

typedef enum { 
  OFFLINE,
  ONLINE
} SENSOR_STATE;

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

// Extern variables
extern const char *sensor_state[];
extern const char *sensor_type[];
extern MULTIPLEXER_STR mux[MUX_CHANNELS];
extern MULTIPLEXER_STR *mc;
extern CH_SENSOR *chs;
extern int site_elevation;

/* 
 *=======================================================================================================================
 * mux_sensor_config() - Substantiate sensors
 *=======================================================================================================================
 */
extern Adafruit_BMP280 bmp1;
extern Adafruit_BMP280 bmp2;
extern Adafruit_BMP280 bmp3;
extern Adafruit_BMP280 bmp4;

extern Adafruit_BME280 bme1;
extern Adafruit_BME280 bme2;
extern Adafruit_BME280 bme3;
extern Adafruit_BME280 bme4;

extern Adafruit_BMP3XX b381;
extern Adafruit_BMP3XX b382;
extern Adafruit_BMP3XX b383;
extern Adafruit_BMP3XX b384;

extern Adafruit_BMP3XX b391;
extern Adafruit_BMP3XX b392;
extern Adafruit_BMP3XX b393;
extern Adafruit_BMP3XX b394;

extern Adafruit_BMP5xx b581;
extern Adafruit_BMP5xx b582;
extern Adafruit_BMP5xx b583;
extern Adafruit_BMP5xx b584;

extern Adafruit_HTU21DF htu1;
extern Adafruit_HTU21DF htu2;
extern Adafruit_HTU21DF htu3;
extern Adafruit_HTU21DF htu4;

extern Adafruit_SHT31 sht31;
extern Adafruit_SHT31 sht32;
extern Adafruit_SHT31 sht33;
extern Adafruit_SHT31 sht34;
extern Adafruit_SHT31 sht35;
extern Adafruit_SHT31 sht36;
extern Adafruit_SHT31 sht37;
extern Adafruit_SHT31 sht38;

extern Adafruit_SHT4x sht41;
extern Adafruit_SHT4x sht42;
extern Adafruit_SHT4x sht43;
extern Adafruit_SHT4x sht44;
extern Adafruit_SHT4x sht45;
extern Adafruit_SHT4x sht46;
extern Adafruit_SHT4x sht47;
extern Adafruit_SHT4x sht48;

extern Adafruit_MCP9808 mcp1;
extern Adafruit_MCP9808 mcp2;
extern Adafruit_MCP9808 mcp3;
extern Adafruit_MCP9808 mcp4;

extern Adafruit_HDC302x hdc1;
extern Adafruit_HDC302x hdc2;
extern Adafruit_HDC302x hdc3;
extern Adafruit_HDC302x hdc4;

extern Adafruit_LPS35HW lps1;
extern Adafruit_LPS35HW lps2;
extern Adafruit_LPS35HW lps3;
extern Adafruit_LPS35HW lps4;

extern Adafruit_SI1145 si1;
extern Adafruit_SI1145 si2;
extern Adafruit_SI1145 si3;
extern Adafruit_SI1145 si4;

extern Adafruit_LTR390 ltr1;
extern Adafruit_LTR390 ltr2;
extern Adafruit_LTR390 ltr3;
extern Adafruit_LTR390 ltr4;

extern LeafSens tlw1;
extern LeafSens tlw2;
extern LeafSens tlw3;
extern LeafSens tlw4;

extern SVCS3 tsm1;
extern SVCS3 tsm2;
extern SVCS3 tsm3;
extern SVCS3 tsm4;

extern BH1750 dfrl1;
extern BH1750 dfrl2;
extern BH1750 dfrl3;
extern BH1750 dfrl4;

// Function prototypes
void mux_sensor_config();
void mux_channel_set(uint8_t i);
void mux_sensor_list();
void mux_sensor_initialize();
void mux_initialize();
bool hih8_getTempHumid(float *t, float *h);

#endif