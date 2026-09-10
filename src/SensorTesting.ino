PRODUCT_VERSION(5);
#define COPYRIGHT "Copyright [2026] [University Corporation for Atmospheric Research]"
#define VERSION_INFO "ICDP_ST-20260715v5"

/*
 *======================================================================================================================
 * ICDP_ST Sensor Testing (ICDP_ST)
 *   Board Type : Particle Boron or Argon
 *   Description: ICDP Sensor Testing using a I2C Multiplexer - TCA9548.
 *                Sensor configuration are to be hard coded in Sensors.h mux_sensor_config() function
 *                Observations are recorded and transmitted every minute.
 *                Reports toParticle as event "ST"
 *   Author: Robert Bubon
 *   Date:  2024-12-27 RJB Initial Development
 *          2025-01-16 RJB Added A0-A5 in the observation output
 *          2025-01-17 RJB Added support for Tinovi Capacitive leaf wetness and Capacitive Soil Moisture sensors
 *          2025-01-22 RJB 1 second observations
 *                         Added support for 4 of each sensor
 *                         Stopped publishing to Particle
 *                         Commented out debug so serial out is only observers when in main loop
 *                         Switched from system clock to rtc clock for time in OBS.h
 *                         Bug fixes
 *          2025-02-13 RJB Bug Fix for SHT31 #4
 *          2025-02-21 RJB Added suport for SHT 5-8
 *          2025-03-03 RJB Added support to log to Particle if OBS_Interval not 0 but a interval time
 *                         Logs to Partice as Event Type "ST"
 *                         Added SI1145 support
 *          2025-03-10 RJB Added support for reporting battery and charging state
 * 
 *          Version 3 Released on 2025-12-15
 *          2025-12-12 RJB OBS_Interval is now seconds [0 to whatever]. A0 results in 1 second obs.
 *                         Logging to Particle only occurs when set to 60 or greater seconds.
 *                         Code clean up
 *                         WatchDog removed
 *                         Sensors added
 *          Version
 *          2026-06-23 RJB Updated to Device OS 6.4.1
 *                         Added DFRobot CO Air sensor - 1s samples reporting a 60s moving average
 *                         Added Sensirion Sen66 Sensor - 1s samples reporting a 60s moving average
 *                         Added BMP581 and SHT45 Sensors        
 *                         Spent a lot of time looking at why serial output stopped. 
 *                           Seems to be tied to the cell modem getting busy and us logging every second.
 *                           When OBS_Interval is less than 60 we do not connect to Particle
 *          2026-08=01 RJB Added Weighing Rain Gauge support
 *                         When scanning the mux channels from background(), the functions called were modified when thay
 *                         call mux_channel_set(c);  It only does this if a sensor is on that channel.
 * 
 * Non-Contact Capacitive leaf wetness, Temperature sensor
 * https://tinovi.com/shop/i2c-non-contact-capacitive-leaf-wetness-temperature/
 *   https://github.com/tinovi/i2cArduino
 *   https://tinovi.com/wp-content/uploads/2021/10/Leaf-Wetness-i2c-2021-10-11.pdf
 *   i2c:0x61
 * 
 * PM-WCS-3-I2C I2C Non-Contact Capacitive Soil Moisture, Temperature sensor
 * https://tinovi.com/shop/i2c-capacitive-soil-moisture-temperature-and-ec-sensor-variation-cable/
 *   https://github.com/tinovi/LeafArduinoI2c
 *   https://tinovi.com/wp-content/uploads/2022/08/PM-WCS-3-I2C.pdf
 *   i2c:0x63
 * 
 * SOIL-MULTI-5-I2C I2C Capacitive multi level soil moisture, temperature sensor
 * https://tinovi.com/shop/soil-multi-5-i2c-i2c-capacitive-soil-moisture-temperature-sensor/
 *   https://github.com/tinovi/i2cMultiSoilArduino/tree/master/lib
 *   https://tinovi.com/wp-content/uploads/2024/07/SOIL-MULTI-5-I2C.pdf
 *   i2c: 0x65
 * 
 * Adafruit STEMMA Soil Sensor - I2C Capacitive Moisture Sensor
 * https://learn.adafruit.com/adafruit-stemma-soil-sensor-i2c-capacitive-moisture-sensor
 * 
 * https://wiki.dfrobot.com/SKU_SEN0562_Gravity_I2C_Waterproof_Ambient_Light_Sensor_1_65535lx
 * https://github.com/claws/BH1750 for include BH1750
 * I2C address 0x23
 * Color 	Label Description
 * Green 	SDA 	I2C Data Input
 * Yellow SCL 	I2C Clock Input
 * Blue 	GND 	Power -
 * Red 	  VCC 	Power +
 * 
 * https://learn.adafruit.com/adafruit-stemma-soil-sensor-i2c-capacitive-moisture-sensor
 * https://github.com/adafruit/Adafruit_Seesaw/tree/master
 * I2C address 0x36
 * 
 * Library Modifications
 *   In DFRobot_MultiGasSensor.h comment out #include "HardwareSerial.h"
 * 
 * 
 */

/* 
 *=======================================================================================================================
 * Local Includes
 *=======================================================================================================================
 */
#include "include/qc.h"             // Quality Control Min and Max Sensor Values on Surface of the Earth
#include "include/support.h"        // Support Functions
#include "include/output.h"         // Serial and OLED Output Functions
#include "include/ps.h"             // Particle Support Functions
#include "include/sdcard.h"         // SD Card
#include "include/analog.h"         // Read Analog Pins
#include "include/time.h"           // Time Management
#include "include/sensors.h"        // I2C Based Sensors
#include "include/obs.h"            // Do Observation Processing
#include "include/wind.h"           // Wind Support Functions
#include "include/dfrgas.h"         // DFRobot Gas Sensors - 1 per mux channel supported
#include "include/dfrwrg.h"         // DFRobot Gravity HX711 Weight Sensor - Weighing Rain Gauge
#include "include/sensirion_sen66.h"// Sensirion sen66 Sensor - 1 per mux channel supported
#include "include/main.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures 
 * =======================================================================================================================
 */
char versioninfo[sizeof(VERSION_INFO)];  // allocate enough space including null terminator
char msgbuf[MAX_MSGBUF_SIZE]; // Used to hold messages
char *msgp;                   // Pointer to message text
char Buffer32Bytes[32];       // General storage
int  LED_PIN = D7;            // Built in LED
int  OBS_Interval=0;          // Values below 60 including 0 will have no logging to Particle
bool JustPoweredOn = true;    // Used to clear SystemStatusBits set during power on device discovery
bool PublishToParticle=true;  // Control if we want to publish to Particle
bool TurnLedOff = false;      // Set true in rain gauge interrupt

bool PostedResults;           // How we did in posting Observation and Need to Send Observations
time32_t Time_of_last_obs = 0;
time32_t Time_of_next_obs = 0;
uint64_t LastTimeUpdate = 0;

#if PLATFORM_ID == PLATFORM_BORON
/*
 * ======================================================================================================================
 *  Power Management IC (bq24195)
 * ======================================================================================================================
 */
PMIC pmic;
#endif              

/*
 * ======================================================================================================================
 * BackGroundWork() - Take Sensor Reading, Check LoRa for Messages, Delay 1 Second for use as timming delay            
 * ======================================================================================================================
 */
void BackGroundWork() {
  // Anything that needs sampling every second add below. Example Wind Speed and Direction, StreamGauge
  Wind_TakeReading();
  dfrgas_TakeReading(); // Goes through all mux channels and takes a reading on the sensor per channel.
  sen66_TakeReading();  // Goes through all mux channels and takes a reading on the sensor per channel.
  dfrwrg_TipCheck();    // Goes through all mux channels and keep an eye out for tips.

  delay (1000);
  if (TurnLedOff) {     // Turned on by rain gauge interrupt handlers
    digitalWrite(LED_PIN, LOW);
    TurnLedOff = false;
  }
}

// You must use SEMI_AUTOMATIC or MANUAL mode so the battery is properly reconnected on
// power-up. If you use AUTOMATIC, you may be unable to connect to the cloud, especially
// on a 2G/3G device without the battery.
SYSTEM_MODE(SEMI_AUTOMATIC);

/*
 * ======================================================================================================================
 * setup() - runs once, when the device is first turned on.
 * ======================================================================================================================
 */
void setup() {

  // Set Default Time Format
  Time.setFormat(TIME_FORMAT_ISO8601_FULL);

  pinMode (LED_PIN, OUTPUT);
  Output_Initialize();
  delay(2000); // Prevents usb driver crash on startup, Arduino needed this so keeping for Particle

  Serial_write(COPYRIGHT);
  Output (VERSION_INFO);
  delay(2000);

  // Initialize SD card if we have one.
  SD_initialize();

  // Check if correct time has been maintained by RTC
  // Uninitialized clock would be 2000-01-00T00:00:00
  stc_timestamp();
  sprintf (msgbuf, "%s+", timestamp);
  Output(msgbuf);

  // Read RTC and set system clock if RTC clock valid
  rtc_initialize();

  if (Time.isValid()) {
    Output("STC: Valid");
  }
  else {
    Output("STC: Not Valid");
  }

  stc_timestamp();
  sprintf (msgbuf, "%s=", timestamp);
  Output(msgbuf);

  Wire.begin();
  dfrgas_setup();
  dfrwrg_setup();
  sen66_setup();   

  mux_initialize();
  analog_initialize();

#if PLATFORM_ID == PLATFORM_ARGON
	pinMode(PWR, INPUT);
	pinMode(CHG, INPUT);
  //==================================================
  // Check if we need to program for WiFi change
  //==================================================
  WiFiPrintCredentials();
  WiFiChangeCheck();
  WiFiPrintCredentials();
#endif

  // See if as5600 is on the i2c main buss. Not the MUX.
  as5600_initialize();

  // Connect the device to the Cloud. 
  // This will automatically activate the cellular connection and attempt to connect 
  // to the Particle cloud if the device is not already connected to the cloud.
  // Upon connection to cloud, time is synced, aka Particle.syncTime()
  if (OBS_Interval >= 60) {
    // Note if we call Particle.connect() and are not truely connected to the Cell network, Code blocks in particle call
    Particle.setDisconnectOptions(CloudDisconnectOptions().graceful(true).timeout(5s));
    Particle.connect();

    // Setup Remote Function to DoAction, Expects a parameter to be passed from Particle to control what action
    if (Particle.function("DoAction", Function_DoAction)) {
      Output ("DoAction:OK");
    }
    else {
      Output ("DoAction:ERR");
    }
  }

  Time_of_next_obs = Time.now() + 60;  // Schedule a obs 60s from now to give network a chance to connect

  Fill_WindGas();

  Output ("LOOP START");
}

/*
 * ======================================================================================================================
 * loop() runs over and over again, as quickly as it can execute.
 * ======================================================================================================================
 */
void loop() {
  BackGroundWork(); // Delays 1 second
  // This will be invalid if the RTC was bad at poweron and we have not connected to Cell network
  // Upon connection to cell network system Time is set and this becomes valid
  if (Time.isValid()) { 
    // Set RTC from Cell network time.
    RTC_UpdateCheck();

    // Perform an Observation, Write to SD, and Transmit observation
    if (Time.now() >= Time_of_next_obs) {
      OBS_Do();
      Time_of_next_obs = Time.now() + OBS_Interval;
    }

    // Request time synchronization from the Cell network - Every 2 Hours
    if ((System.millis() - LastTimeUpdate) > (2*3600*1000)) {
      if (Particle.connected()) {
        // Note that this function sends a request message to the Cloud and then returns. 
        // The time on the device will not be synchronized until some milliseconds later when 
        // the Cloud responds with the current time between calls to your loop.

        // !!! What if we drop the Cell connection before we get a time update for the Cloud?
        Output ("NW TimeSync REQ");
        Particle.syncTime();
      }
      else {
        Output ("NW TimeSync NC!"); // Not Connected
      }
      LastTimeUpdate = System.millis();
    } 
  }
  else {
    stc_timestamp();
    Output(timestamp);
    Output("ERR: No Clock");
    delay (DELAY_NO_RTC);   
  }
}