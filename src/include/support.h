/*
 * ======================================================================================================================
 *  support.h - Support Functions Definations
 * ======================================================================================================================
 */
#include <Particle.h>

// Function prototypes
bool I2C_Device_Exist(byte address);
void Blink(int count, int between);
void mysort(unsigned int a[], unsigned int n);

#if PLATFORM_ID == PLATFORM_ARGON
void WiFiChangeCheck();
void WiFiPrintCredentials();
#endif