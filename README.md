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
2026-07-15:
- Added Sensirion Sen66 Sensor:
  - One sensor per mux channel support - 1s samples reporting a 60s moving average
  - Tag name: s66-tag-id number specified in mux_sensor_config().
  - Example: s66-Pm4p0-1, s66-Pm10p0-1, s66-h-1, s66-t-1, s66-voc-1, s66-nox-1, s66-co2-1

2026-06-30:
- Added DFRobot Gas Sensors: o2, co, h2s, no2, o3, cl2, nh3, h2, hcl, so2, hf, ph
  - One sensor per mux channel support - 1s samples reporting a 60s moving average
  - Tag name: tfrg-<sensor type>-id number specified in mux_sensor_config().
  - Example: dfrg-co-1

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
09:41:59.302 -> OLED:Enabled
09:41:59.373 -> SC:Enabled
09:41:59.440 -> SER:OK
09:42:01.434 -> Copyright [2026] [University Corporation for Atmospheric Research]
09:42:01.508 -> ICDP_ST-20260623v4
09:42:03.585 -> SD:Online
09:42:03.653 -> SD:OBS DIR Exists
09:42:03.725 -> 2000-01-01T00:00:07+
09:42:03.797 -> 2026-07-15T15:42:06*
09:42:03.869 -> RTC:VALID
09:42:03.902 -> STC: Valid
09:42:03.969 -> 2026-07-15T15:42:06=
09:42:04.038 -> MUX:INIT
09:42:04.110 -> MUX:SENSOR:LIST
09:42:04.182 -> CH:0 S:0,mcp1,0x18,OFFLINE
09:42:04.255 -> CH:0 S:1,dfrg1,0x74,OFFLINE
09:42:04.328 -> CH:1 S:0,s661,0x6b,OFFLINE
09:42:04.362 -> CH:1 S:1,hdc1,0x46,OFFLINE
09:42:04.430 -> MUX:SENSOR:INIT
09:42:04.503 -> CH:0 S:0,mcp1,0x18
09:42:04.575 -> OFFLINE
09:42:04.649 -> CH:0 S:1,dfrg1,0x74
09:42:04.720 ->   INITIALIZING
09:42:04.795 ->   AQUIRE MODE:INITIATIVE
09:42:04.867 ->   TEMP COMPENSATION:ON
09:42:04.936 ->   TYPE:co
09:42:05.041 ->   SAMPLE:0.00 PPM
09:42:05.075 ->   ONLINE
09:42:05.146 -> CH:1 S:0,s661,0x6b
09:42:05.215 ->   INITIALIZING
09:42:07.694 ->   ALT:1546m
09:42:07.827 ->   CONT.MEASURE:SET
09:42:07.900 ->   PRODUCT:SEN66
09:42:08.014 ->   TYPE:00085300
09:42:08.084 ->   SN:7B7C28564FE99A8D
09:42:08.195 ->   FW:4.0
09:42:08.231 -> DELAY 10s For SEN66 To Become Ready
09:42:18.348 ->   Pm1p0:126.80
09:42:18.417 ->   Pm2p5:135.60
09:42:18.487 ->   Pm4p0:138.60
09:42:18.522 ->   Pm10p0:140.10
09:42:18.597 ->   Humidity:35.82
09:42:18.672 ->   Temp:24.08
09:42:18.747 ->   VOC:0.00
09:42:18.782 ->   NOX:0.00
09:42:18.854 ->   CO2:518
09:42:18.928 ->   ONLINE
09:42:19.002 -> CH:1 S:1,hdc1,0x46
09:42:19.073 -> ONLINE
09:42:19.142 -> MUX:INIT EXIT
09:42:19.217 -> AS5600:INIT
09:42:19.289 -> WD:NF
09:42:19.362 -> Fill Wind&Gas
09:42:19.431 -> DFRGAS Take Reading
09:42:19.500 -> SEN66 Take Reading
09:43:18.535 -> SEN66 Take Reading
09:43:19.426 -> LOOP START
09:43:20.479 -> {"at":"2026-07-15T15:43:23","epoch":1784130203,"uptime":84,"bcs":6,"bpc":0.00,"cfr":0,"dfrg-co-1":0.00,"s66-Pm1p0-1":128.01,"s66-Pm2p5-1":133.15,"s66-Pm4p0-1":133.15,"s66-Pm10p0-1":133.16,"s66-h-1":36.61,"s66-t-1":23.13,"s66-voc-1":13.75,"s66-nox-1":0.30,"s66-co2-1":620,"hdc1t":21.76,"hdc1h":42.63}
09:43:21.619 -> {"at":"2026-07-15T15:43:24","epoch":1784130204,"uptime":85,"bcs":6,"bpc":0.00,"cfr":0,"dfrg-co-1":0.00,"s66-Pm1p0-1":127.64,"s66-Pm2p5-1":132.75,"s66-Pm4p0-1":132.75,"s66-Pm10p0-1":132.75,"s66-h-1":36.68,"s66-t-1":23.08,"s66-voc-1":14.93,"s66-nox-1":0.32,"s66-co2-1":622,"hdc1t":21.78,"hdc1h":42.62}
09:43:22.785 -> {"at":"2026-07-15T15:43:25","epoch":1784130205,"uptime":87,"bcs":6,"bpc":0.00,"cfr":0,"dfrg-co-1":0.00,"s66-Pm1p0-1":128.74,"s66-Pm2p5-1":133.89,"s66-Pm4p0-1":133.89,"s66-Pm10p0-1":133.89,"s66-h-1":36.75,"s66-t-1":23.04,"s66-voc-1":16.22,"s66-nox-1":0.33,"s66-co2-1":623,"hdc1t":21.78,"hdc1h":42.64}
</pre>