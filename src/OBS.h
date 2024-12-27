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

  // Safty Check for Vaild Time
  if (!Time.isValid()) {
    Output ("OBS_Do: Time NV");
    return;
  }

  Output("OBS_Do:");

  stc_timestamp();
  Output(timestamp);

  memset(msgbuf, 0, sizeof(msgbuf));
  JSONBufferWriter writer(msgbuf, sizeof(msgbuf)-1);

  writer.beginObject();
  writer.name("at").value(timestamp);

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
                    dt = (isnan(dt) || (t < QC_MIN_T)  || (dt > QC_MAX_T))  ? QC_ERR_T  : dt;
                    dh = (isnan(dh) || (h < QC_MIN_RH) || (dh > QC_MAX_RH)) ? QC_ERR_RH : dh;
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

                default :
                  Output ("Invalid Sensor ID");
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

  writer.endObject();

  // Log Observation to SD Card
  SD_LogObservation(msgbuf);
  Serial_write (msgbuf);

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

