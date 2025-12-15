/*
 * ======================================================================================================================
 *  ps.h - Particle Support Definations
 * ======================================================================================================================
 */

// Extern variables
extern const char* pinNames[];

// Function prototype
void GetPinName(pin_t pin, char *pinname);
void DeviceReset();
int Function_DoAction(String s);

