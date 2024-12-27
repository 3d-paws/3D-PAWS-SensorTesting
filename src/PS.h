/*
 * ======================================================================================================================
 * PS.h - Particle Support Funtions
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 * DeviceReset() - Kill power to ourselves and do a cold boot
 * ======================================================================================================================
 */
void DeviceReset() {
  digitalWrite(REBOOT_PIN, HIGH);
  delay(5000);
  // Should not get here if relay / watchdog is connected.
  digitalWrite(REBOOT_PIN, LOW);
  delay(2000); 

  // May never get here if relay board / watchdog not connected.

  // Resets the device, just like hitting the reset button or powering down and back up.
  System.reset();
}

/*
 * ======================================================================================================================
 * Function_DoAction() - Handler for Particle Function DoAction     
 * ======================================================================================================================
 */
int Function_DoAction(String s) {
  if (strcmp (s,"REBOOT") == 0) {  // Reboot
    Output("DoAction:REBOOT");
    delay(1000);

    DeviceReset();

    // Never gets here and thus can never send a ack to Particle Portal
    return(0);  
  }
  else {
    Output("DoAction:UKN"); 
    return(-1);
  }
}
