# ICDP Sensor Testing (ICDP_ST)

Board Type: Particle Boron or Argon

## Description
ICDP Sensor Testing using an I2C Multiplexer - TCA9548.

- Sensor configurations must be hand-coded in the Sensors.h mux_sensor_config() function
- Observations are recorded and transmitted every second
- Data is logged to SD card

## Features
- Support for multiple sensors including MCP9808, BME280, HDC1080, and LPS22HB
- OLED display support (currently disabled)
- SD card logging
- RTC synchronization
- WiFi connectivity

## Recent Updates
- 2025-01-22: Implemented 1-second observations
- Added support for 4 of each sensor type
- Stopped publishing to Particle, now only logging to SD card
- Switched from system clock to RTC clock for time in OBS.h
- Network time sync every 2 hours instead of 4

## Example Serial Output
<pre style="font-size: 0.8em;">
OLED:Disabled
SC:Enabled
SER:OK
Copyright [2024] [University Corporation for Atmospheric Research]
ICDP_ST-20241227
SETUP WATCHDOG PINs
SD:Online
SD:OBS DIR Exists
2000-01-01T00:00:33+
2025-01-23T17:35:31*
RTC:VALID
STC: Valid
2025-01-23T17:35:31=
MUX:INIT
MUX:SENSOR:LIST
CH:0 S:0,mcp1,0x18,OFFLINE
CH:1 S:0,b391,0x77,OFFLINE
CH:2 S:0,hdc1,0x46,OFFLINE
CH:2 S:1,hdc2,0x47,OFFLINE
CH:3 S:0,lps1,0x5d,OFFLINE
CH:4 S:0,tlw1,0x61,OFFLINE
CH:4 S:1,tsm1,0x63,OFFLINE
MUX:SENSOR:INIT
  CH:0 S:0,mcp1,0x18
    ONLINE
  CH:1 S:0,b391,0x77
    ONLINE
  CH:2 S:0,hdc1,0x46
    ONLINE
  CH:2 S:1,hdc2,0x47
    ONLINE
  CH:3 S:0,lps1,0x5d
    ONLINE
  CH:4 S:0,tlw1,0x61
    OFFLINE
  CH:4 S:1,tsm1,0x63
    OFFLINE
MUX:INIT EXIT
WIFI MAC[e8:9f:6d:be:6d:28]
WIFI Creds:
IDX[0]
 SSID[-]
 Type:WPA2
 SigStr[0]
 Channel[0]
 BMAC[cc:40:d0:87:3a:9f]
WIFI:Open
WIFI:ATYPE[WPA2]
WIFI:SSID[......]
WIFI:PW[........]
WIFI:Credentials Cleared
WIFI:Credentials Set WPA2
WIFI MAC[e8:9f:6d:be:6d:28]
WIFI Creds:
IDX[0]
 SSID[-]
 Type:WPA2
 SigStr[0]
 Channel[0]
 BMAC[ff:ff:ff:ff:ff:ff]
DoAction:OK
LOOP START
{"at":"2025-01-23T17:35:33","epoch":1737653733,"mcp1t":20.12,"b391t":20.52,"b391p":854.0941,"hdc1t":20.40,"hdc1h":28.70,"hdc2t":20.10,"hdc2h":29.37,"lps1t":14.14,"lps1p":853.6233}
{"at":"2025-01-23T17:35:34","epoch":1737653734,"mcp1t":20.19,"b391t":20.53,"b391p":854.0617,"hdc1t":20.40,"hdc1h":28.70,"hdc2t":20.10,"hdc2h":29.40,"lps1t":14.14,"lps1p":853.6223}
{"at":"2025-01-23T17:35:35","epoch":1737653735,"mcp1t":20.19,"b391t":20.53,"b391p":854.1168,"hdc1t":20.40,"hdc1h":28.71,"hdc2t":20.10,"hdc2h":29.42,"lps1t":14.14,"lps1p":853.6265}
{"at":"2025-01-23T17:35:36","epoch":1737653736,"mcp1t":20.19,"b391t":20.53,"b391p":854.0617,"hdc1t":20.38,"hdc1h":28.71,"hdc2t":20.12,"hdc2h":29.43,"lps1t":14.15,"lps1p":853.6387}
{"at":"2025-01-23T17:35:37","epoch":1737653737,"mcp1t":20.19,"b391t":20.53,"b391p":854.1010,"hdc1t":20.40,"hdc1h":28.71,"hdc2t":20.10,"hdc2h":29.45,"lps1t":14.14,"lps1p":853.6187}
{"at":"2025-01-23T17:35:38","epoch":1737653738,"mcp1t":20.19,"b391t":20.53,"b391p":854.0134,"hdc1t":20.38,"hdc1h":28.70,"hdc2t":20.12,"hdc2h":29.44,"lps1t":14.15,"lps1p":853.6208}
{"at":"2025-01-23T17:35:39","epoch":1737653739,"mcp1t":20.19,"b391t":20.53,"b391p":854.0941,"hdc1t":20.40,"hdc1h":28.72,"hdc2t":20.12,"hdc2h":29.45,"lps1t":14.15,"lps1p":853.6289}
{"at":"2025-01-23T17:35:40","epoch":1737653740,"mcp1t":20.19,"b391t":20.53,"b391p":854.0460,"hdc1t":20.40,"hdc1h":28.71,"hdc2t":20.12,"hdc2h":29.43,"lps1t":14.15,"lps1p":853.6252}
{"at":"2025-01-23T17:35:41","epoch":1737653741,"mcp1t":20.19,"b391t":20.54,"b391p":854.1138,"hdc1t":20.40,"hdc1h":28.70,"hdc2t":20.12,"hdc2h":29.40,"lps1t":14.15,"lps1p":853.6409}
{"at":"2025-01-23T17:35:42","epoch":1737653742,"mcp1t":20.19,"b391t":20.53,"b391p":854.0685,"hdc1t":20.40,"hdc1h":28.70,"hdc2t":20.15,"hdc2h":29.38,"lps1t":14.15,"lps1p":853.6487}
{"at":"2025-01-23T17:35:43","epoch":1737653743,"mcp1t":20.19,"b391t":20.54,"b391p":854.0912,"hdc1t":20.40,"hdc1h":28.68,"hdc2t":20.12,"hdc2h":29.36,"lps1t":14.15,"lps1p":853.6287}
RTC: 1ST SYNC
2025-01-23T17:35:44*
{"at":"2025-01-23T17:35:44","epoch":1737653744,"mcp1t":20.19,"b391t":20.54,"b391p":854.0460,"hdc1t":20.38,"hdc1h":28.69,"hdc2t":20.12,"hdc2h":29.36,"lps1t":14.15,"lps1p":853.6094}</pre>