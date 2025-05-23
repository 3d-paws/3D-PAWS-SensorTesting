/*
 * ======================================================================================================================
 *  OBS.h - Observation Handeling
 * ======================================================================================================================
 */

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

            // BMP
            case bmp :
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

            // BME
            case bme :
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

            // B38
            case b38 :
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

            // B39
            case b39 :
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

            // HTU
            case htu :
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

            // SHT
            case sht :
              switch (chs->id) {
                case 1 :
                  t = sht1.readTemperature();
                  h = sht1.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht1t").value(t, 2);
                  writer.name("sht1h").value(h, 2);
                  break;

                case 2 :
                  t = sht2.readTemperature();
                  h = sht2.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht2t").value(t, 2);
                  writer.name("sht2h").value(h, 2);
                  break;

                case 3 :
                  t = sht3.readTemperature();
                  h = sht3.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht3t").value(t, 2);
                  writer.name("sht3h").value(h, 2);
                  break;

                case 4 :
                  t = sht4.readTemperature();
                  h = sht4.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht4t").value(t, 2);
                  writer.name("sht4h").value(h, 2);
                  break;

                case 5 :
                  t = sht5.readTemperature();
                  h = sht5.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht5t").value(t, 2);
                  writer.name("sht5h").value(h, 2);
                  break;

                case 6 :
                  t = sht6.readTemperature();
                  h = sht6.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht6t").value(t, 2);
                  writer.name("sht6h").value(h, 2);
                  break;

                case 7 :
                  t = sht7.readTemperature();
                  h = sht7.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht7t").value(t, 2);
                  writer.name("sht7h").value(h, 2);
                  break;

                case 8 :
                  t = sht8.readTemperature();
                  h = sht8.readHumidity();
                  t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
                  h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
                  writer.name("sht8t").value(t, 2);
                  writer.name("sht8h").value(h, 2);
                  break;

                default :
                  Output ("Invalid Sensor ID");
                  break;
              }
              break;

            // MCP
            case mcp :
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

            // HDC
            case hdc :
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

            // LPS
            case lps :
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

            // HIH
            case hih :
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

            // Tinovi Leaf Wetness
            case tlw :
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

            // Tinovi Soil Moisture
            case tsm :
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

            // Si1145 UV/IR/Visible Light Sensor
            case si :
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

            // Default - Sensor tyoe not found
            default :
              break;
          }
        }
      }
    }
  }

  // Read and report Analog Pins
  for (int pin=0; pin<ANALOG_PIN_COUNT; pin++) {
    if (analog_pins[pin].inuse) {
      char tag[16];

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

  if (OBS_Interval) { // If not set to 0 (1 second observations) sned to Particle
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

