PRODUCT_VERSION(1);
#define COPYRIGHT "Copyright [2024] [University Corporation for Atmospheric Research]"
#define VERSION_INFO "ICDP_ST-20250124"

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
 *  
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
 */

#define W4SC false   // Set true to Wait for Serial Console to be connected

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_MCP9808.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_HDC302x.h>
#include <Adafruit_LPS35HW.h>
#include <i2cArduino.h>
#include <LeafSens.h>

#include <RTClib.h>
#include <SdFat.h>

/*
 * ======================================================================================================================
 *  Loop Timers
 * ======================================================================================================================
 */
#define DELAY_NO_RTC              1000*60    // Loop delay when we have no valided RTC
#define CLOUD_CONNECTION_TIMEOUT  90         // Wait for N seconds to connect to the Cell Network

/*
 * ======================================================================================================================
 *  Relay Power Control Pin
 * ======================================================================================================================
 */
#define REBOOT_PIN        A0  // Toggle power via relay board
#define HEARTBEAT_PIN     A1  // Connect to PICAXE-8M PIN-C3
#define MAX_MSGBUF_SIZE 1024

/*
 * ======================================================================================================================
 *  Globals
 * ======================================================================================================================
 */
char msgbuf[MAX_MSGBUF_SIZE]; // Used to hold messages
char *msgp;                   // Pointer to message text
char Buffer32Bytes[32];       // General storage
int  LED_PIN = D7;            // Built in LED
bool TurnLedOff = false;      // Set true in rain gauge interrupt
bool JustPoweredOn = true;    // Used to clear SystemStatusBits set during power on device discovery
bool PostedResults;           // How we did in posting Observation and Need to Send Observations

time32_t Time_of_last_obs = 0;
time32_t Time_of_next_obs = 0;

int countdown = 1800;         // Exit calibration mode when reaches 0 - protects against burnt out pin or forgotten jumper
uint64_t LastTimeUpdate = 0;
uint64_t StartedConnecting = 0;
bool ParticleConnecting = false;
bool TakeObservation = true;  // When set we take OBS and transmit it
bool PowerDown = false;

#if PLATFORM_ID == PLATFORM_BORON
const char* pinNames[] = {
    "A0", "A1", "A2", "A3", "A4", "A5",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8",
    "D9", "D10", "D11", "D12", "D13", "D14", "D15",
    "SDA", "SCL", "TX", "RX", "MISO", "MOSI", "SCK", "SS",
    "WKP", "VUSB", "Li+"
};
#endif

#if PLATFORM_ID == PLATFORM_ARGON
const char* pinNames[] = {
    "A0", "A1", "A2", "A3", "A4", "A5",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8",
    "D9", "D10", "D11", "D12", "D13", "D14", "D15", "D16", "D17", "D18", "D19",
    "SDA", "SCL", "TX", "RX", "MISO", "MOSI", "SCK", "SS",
    "WKP", "VUSB", "Li+", "EN", "3V3", "GND"
};
#endif

/*
 * ======================================================================================================================
 * International Mobile Subscriber Identity
 * ======================================================================================================================
 */
char imsi[16] = "";
bool imsi_valid = false;
time32_t imsi_next_try = 0;

/*
 * ======================================================================================================================
 *  SD Card
 * ======================================================================================================================
 */
#define SD_ChipSelect D5                // GPIO 10 is Pin 10 on Feather and D5 on Particle Boron Board
SdFat SD;                               // File system object.
File SD_fp;
char SD_obsdir[] = "/OBS";              // Store our obs in this directory. At Power on, it is created if does not exist
bool SD_exists = false;                     // Set to true if SD card found at boot

char SD_wifi_file[] = "WIFI.TXT";       // File used to set WiFi configuration

/*
 * ======================================================================================================================
 *  Local Code Includes - Do not change the order of the below 
 * ======================================================================================================================
 */
#include "QC.h"                   // Quality Control Min and Max Sensor Values on Surface of the Earth
#include "SF.h"                   // Support Functions
#include "OP.h"                   // OutPut support for OLED and Serial Console
#include "TM.h"                   // Time Management
#include "Sensors.h"              // I2C Based Sensors
#include "Analog.h"               // Read Analog Pins
#include "SDC.h"                  // SD Card
#include "OBS.h"                  // Do Observation Processing
#include "PS.h"                   // Particle Support Functions

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
 * HeartBeat() - 
 * ======================================================================================================================
 */
void HeartBeat() {
  digitalWrite(HEARTBEAT_PIN, HIGH);
  delay(250);
  digitalWrite(HEARTBEAT_PIN, LOW);
}

/*
 * ======================================================================================================================
 * BackGroundWork() - Take Sensor Reading, Check LoRa for Messages, Delay 1 Second for use as timming delay            
 * ======================================================================================================================
 */
void BackGroundWork() {
  // Anything that needs sampling every second add below. Example Wind Speed and Direction, StreamGauge
  HeartBeat();  // 250ms
  delay (750);
  if (TurnLedOff) {     // Turned on by rain gauge interrupt handlers
    digitalWrite(LED_PIN, LOW);
    TurnLedOff = false;
  }
}

// You must use SEMI_AUTOMATIC or MANUAL mode so the battery is properly reconnected on
// power-up. If you use AUTOMATIC, you may be unable to connect to the cloud, especially
// on a 2G/3G device without the battery.
SYSTEM_MODE(SEMI_AUTOMATIC);

// https://docs.particle.io/cards/firmware/system-thread/system-threading-behavior/
SYSTEM_THREAD(ENABLED);

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
  delay(4000);

  // WatchDog - By default all pins are LOW when board is first powered on. Setting OUTPUT keeps pin LOW.
  Output ("SETUP WATCHDOG PINs");  // 
  pinMode (REBOOT_PIN, OUTPUT);
  pinMode (HEARTBEAT_PIN, OUTPUT);

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

  // Connect the device to the Cloud. 
  // This will automatically activate the cellular connection and attempt to connect 
  // to the Particle cloud if the device is not already connected to the cloud.
  // Upon connection to cloud, time is synced, aka Particle.syncTime()

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

  Time_of_next_obs = Time.now() + 60;  // Schedule a obs 60s from now to give network a chance to connect

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
    OBS_Do();

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