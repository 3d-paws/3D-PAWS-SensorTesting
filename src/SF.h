/*
 * ======================================================================================================================
 *  SF.h - Support Functions
 * ======================================================================================================================
 */

// Prototyping functions to aviod compile function unknown issue.
void Output(const char *str);

/* 
 *=======================================================================================================================
 * I2C_Device_Exist - does i2c device exist
 * 
 *  The i2c_scanner uses the return value of the Write.endTransmisstion to see 
 *  if a device did acknowledge to the address.
 *=======================================================================================================================
 */
bool I2C_Device_Exist(byte address) {
  byte error;

  Wire.begin();                     // Connect to I2C as Master (no addess is passed to signal being a slave)

  Wire.beginTransmission(address);  // Begin a transmission to the I2C slave device with the given address. 
                                    // Subsequently, queue bytes for transmission with the write() function 
                                    // and transmit them by calling endTransmission(). 

  error = Wire.endTransmission();   // Ends a transmission to a slave device that was begun by beginTransmission() 
                                    // and transmits the bytes that were queued by write()
                                    // By default, endTransmission() sends a stop message after transmission, 
                                    // releasing the I2C bus.

  // endTransmission() returns a byte, which indicates the status of the transmission
  //  0:success
  //  1:data too long to fit in transmit buffer
  //  2:received NACK on transmit of address
  //  3:received NACK on transmit of data
  //  4:other error 

  // Partice Library Return values
  // SEE https://docs.particle.io/cards/firmware/wire-i2c/endtransmission/
  // 0: success
  // 1: busy timeout upon entering endTransmission()
  // 2: START bit generation timeout
  // 3: end of address transmission timeout
  // 4: data byte transfer timeout
  // 5: data byte transfer succeeded, busy timeout immediately after
  // 6: timeout waiting for peripheral to clear stop bit

  if (error == 0) {
    return (true);
  }
  else {
    // sprintf (msgbuf, "I2CERR: %d", error);
    // Output (msgbuf);
    return (false);
  }
}

/*
 * ======================================================================================================================
 * Blink() - Count, delay between, delay at end
 * ======================================================================================================================
 */
void Blink(int count, int between)
{
  int c;

  for (c=0; c<count; c++) {
    digitalWrite(LED_PIN, HIGH);
    delay(between);
    digitalWrite(LED_PIN, LOW);
    delay(between);
  }
}

/*
 * ======================================================================================================================
 * myswap()
 * ======================================================================================================================
 */
void myswap(unsigned int *p, unsigned int *q) {
  int t;
   
  t=*p; 
  *p=*q; 
  *q=t;
}

/*
 * ======================================================================================================================
 * mysort()
 * ======================================================================================================================
 */
void mysort(unsigned int a[], unsigned int n) { 
  unsigned int i, j;

  for(i = 0;i < n-1;i++) {
    for(j = 0;j < n-i-1;j++) {
      if(a[j] > a[j+1])
        myswap(&a[j],&a[j+1]);
    }
  }
}

#if PLATFORM_ID == PLATFORM_ARGON
/*
 * ======================================================================================================================
 * WiFiChangeCheck() - Check for WIFI.TXT file and set Wireless SSID, Password            
 * ======================================================================================================================
 */
void WiFiChangeCheck() {
  File fp;
  int i=0;
  char *p, *auth, *ssid, *pw;
  char ch, buf[128];
  bool changed = false;

  if (SD_exists) {
    // Test for file WIFI.TXT
    if (SD.exists(SD_wifi_file)) {
      fp = SD.open(SD_wifi_file, FILE_READ); // Open the file for reading, starting at the beginning of the file.

      if (fp) {
        // Deal with too small or too big of file
        if (fp.size()<=7 || fp.size()>127) {
          fp.close();
          Output ("WIFI:Invalid SZ");
        }
        else {
          Output ("WIFI:Open");
          // Read one line from file
          while (fp.available() && (i < 127 )) {
            ch = fp.read();

            // sprintf (msgbuf, "%02X : %c", ch, ch);
            // Output (msgbuf);

            if ((ch == 0x0A) || (ch == 0x0D) ) {  // newline or linefeed
              break;
            }
            else {
              buf[i++] = ch;
            }
          }
          fp.close();

          // At this point we have encountered EOF, CR, or LF
          // Now we need to terminate array with a null to make it a string
          buf[i] = (char) NULL;

          // Parse string for the following
          //   WIFI ssid password
          p = &buf[0];
          auth = strtok_r(p, ",", &p);

          if (auth == NULL) {
            Output("WIFI:ID=Null Err");
          }
          else if ( (strcmp (auth, "WEP") != 0)  &&
                    (strcmp (auth, "WPA") != 0)  &&
                    (strcmp (auth, "WPA2") != 0) &&
                    (strcmp (auth, "UNSEC") != 0)) {
            sprintf (msgbuf, "WIFI:ATYPE[%s] Err", auth);          
            Output(msgbuf);
          }
          else {
            ssid = strtok_r(p, ",", &p);
            pw  = strtok_r(p, ",", &p);
            
            if (pw == NULL) {
              pw = (char *) "";  // Handle the case when nothing is after the ","
            }

            if (ssid == NULL) {
              Output("WIFI:SSID=Null Err");
            }

            // UNSEC is allow to have no password just a ssid
            else if ((strcmp (auth, "UNSEC") != 0)  && (pw == NULL)) {
              Output("WIFI:PW=Null Err");
            }
            else {
              sprintf (msgbuf, "WIFI:ATYPE[%s]", auth);          
              Output(msgbuf);
              sprintf (msgbuf, "WIFI:SSID[%s]", ssid);
              Output(msgbuf);
              sprintf (msgbuf, "WIFI:PW[%s]", pw);
              Output(msgbuf);

              // Connects to a network secured with WPA2 credentials.
              // https://docs.particle.io/reference/device-os/api/wifi/securitytype-enum/
              if (strcmp (auth, "UNSEC") == 0) {
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                if (strcmp (pw, "") == 0) {
                  Output("WIFI:Credentials Set UNSEC NO PW");
                  WiFi.setCredentials(ssid);
                }
                else {
                  Output("WIFI:Credentials Set UNSEC");
                  WiFi.setCredentials(ssid, pw);                 
                }
              }
              else if (strcmp (auth, "WEP") == 0) {
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                Output("WIFI:Credentials Set WEP");
                WiFi.setCredentials(ssid, pw, WEP);
              }
              else if (strcmp (auth, "WPA") == 0) {
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                Output("WIFI:Credentials Set WPA");
                WiFi.setCredentials(ssid, pw, WPA);
              }
              else if (strcmp (auth, "WPA2") == 0) {
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                Output("WIFI:Credentials Set WPA2");
                WiFi.setCredentials(ssid, pw, WPA2);
              }
              else if (strcmp (auth, "WPA_ENTERPRISE") == 0) {
                // WPA Enterprise is only supported on the Photon and P1.
                // It is not supported on the Argon, P2, and Photon 2.
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                Output("WIFI:Credentials Set WPAE");
                WiFi.setCredentials(ssid, pw, WPA_ENTERPRISE);
              }
              else if (strcmp (auth, "WPA2_ENTERPRISE") == 0) {
                // WPA Enterprise is only supported on the Photon and P1.
                // It is not supported on the Argon, P2, and Photon 2.
                Output("WIFI:Credentials Cleared");
                WiFi.clearCredentials();
                Output("WIFI:Credentials Set WPAE2");
                WiFi.setCredentials(ssid, pw, WPA2_ENTERPRISE);
              }
              else { 
                Output("WIFI:Credentials NOT Set");
                Output("WIFI:USING NVAUTH");
              }
            }
          }
        }
      }
      else {
        sprintf (msgbuf, "WIFI:Open[%s] Err", SD_wifi_file);          
        Output(msgbuf);
        Output ("WIFI:USING NVAUTH");
      }
    } 
    else {
      Output ("WIFI:NOFILE USING NVAUTH");
    }
  } // SD enabled
  else {
    Output ("WIFI:NOSD USING NVAUTH");
  }
}

/*
 * ======================================================================================================================
 * WiFiPrintCredentials() - Read NVRAM and print WiFi Creditials     
 * 
 * Particle's Device OS does not provide a direct method to determine which of the stored Wi-Fi credentials is currently 
 * being used for the connection. However, you can infer this indirectly by attempting to connect to each stored Wi-Fi 
 * network and checking if the connection is successful.        
 * ======================================================================================================================
 */
void WiFiPrintCredentials() {
  byte mac[6];

  WiFi.macAddress(mac);


  sprintf (msgbuf, "WIFI MAC[%02x:%02x:%02x:%02x:%02x:%02x]", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Output(msgbuf);

  WiFiAccessPoint ap[5];
  WiFi.getCredentials(ap, 5);
  int found = 0;

  for (int i = 0; i < 5; i++) {
    if (strlen(ap[i].ssid) != 0) {
      found++;
      if (found == 1) {
        Output("WIFI Creds:");
      }
      sprintf (msgbuf, "IDX[%d]", i);
      Output(msgbuf);
      sprintf (msgbuf, " SSID[%s]", ap[i].ssid);
      Output(msgbuf);
      switch (ap[i].security) {
        case WLAN_SEC_UNSEC:
          Output(" Type:Unsecured");
          break;
        case WLAN_SEC_WEP:
          Output(" Type:WEP");
          break;
        case WLAN_SEC_WPA:
          Output(" Type:WPA");
          break;
        case WLAN_SEC_WPA2:
          Output(" Type:WPA2");
          break;
        case WLAN_SEC_WPA_ENTERPRISE:
          Output(" Type:WPA Ent");
          break;
        case WLAN_SEC_WPA2_ENTERPRISE:
          Output(" Type:WPA2 Ent");
          break;
        default:
          Output(" Type:Unknown");
          break;
      }
      sprintf (msgbuf, " SigStr[%d]", ap[i].rssi);
      Output(msgbuf);
      sprintf (msgbuf, " Channel[%d]", ap[i].channel);
      Output(msgbuf);
      sprintf (msgbuf, " BMAC[%02x:%02x:%02x:%02x:%02x:%02x]", 
        ap[i].bssid[0], ap[i].bssid[1], ap[i].bssid[2], ap[i].bssid[3], ap[i].bssid[4], ap[i].bssid[5]);
      Output(msgbuf);
    }
  }
  if (!found) {
    Output("WIFI Creds:NF");
  }
}
#endif