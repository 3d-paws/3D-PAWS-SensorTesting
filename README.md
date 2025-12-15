# ICDP Sensor Testing (ICDP_ST)

## Features

- Particle Boron or Argon support
- Support for multiple sensors of the same type supported via i2c mux.
- SD card logging
- RTC synchronization
- WiFi connectivity

## Sensors 

Configurations must be hand-coded and the code compiled.

I2C Sensors
- I2C Sensors must be connected to the I2C Multiplexer - TCA9548.
- Configuring i2c sensors, edit file "sensors.cpp" and the "mux_sensor_config()" function.
- Example configurations left in place should be commented out or removed.

Analog Sensors
- Configuring analog sensors edit the "analog.cpp" file and the "analog_initialize()" function.
- Example configurations left in place should be commented out or removed.

Wind Sensor
- To enable, add a AS5600 to the main i2c buss. Not the multiplexer.
- If the AS5600 is discoved at boot, pin A2 will be used for wind speed. 
- When wind is enabled the analog sensor on pin 2 is disabled.

## Setting the Observation Period.

- For controlling the observation period is done by editing the "SensorTesting.cpp" file variable "OBS_Interval" around line 87.
- This value is in seconds.
- A value below 59 will mean no observation will be logged to Particle. Only local logging.

## Local Logging

- Local logging is done to the SD card.
- To log to the serial console set the Serial Console pin D8 to ground at startup. If no serial console is connected to the usb port in 60 seconds from boot. The boot will continue.
- OLED display is enabled if OLED discovered at boot.

## Particle Logging

- Event type "ST" is used when logging to Particle.

## Recent Notes

2025-12-15:

- Removed WatchDog support.  You can still do a soft reboot from Particle console. This just reboot the particle board.
- Added wind support.
- Add sensors
  - Adafruit BMP581
  - Adafruit SHT45
  - Adafruit LTR390 UV Light Sensor
  - DFRobot SEN0562 Ambient Light Sensor 1-65535lx BH1750
- Changed interval time to be any amount of seconds that is configured.
- Code clean, .cpp files.
- Changed how observation period handling to be just configured seconds.
- Note: Adafruit Soil Sensor - Part Number 4026 has i2c address 0x36 which same as AS5600. We will not support this sensor.

2025-01-22: 

- Implemented 1-second observations
- Added support for 4 of each sensor type (8 for SHT sensors)
- Stopped publishing to Particle, now only logging to SD card
- Switched from system clock to RTC clock for time in OBS.h
- Network time sync every 2 hours instead of 4

## Example Serial Output
<pre style="font-size: 0.8em;">
</pre>