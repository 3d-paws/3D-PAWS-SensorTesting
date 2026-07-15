/*
 * ======================================================================================================================
 *  sdcard.h - SD Card Definations
 * ======================================================================================================================
 */
#include <SdFat.h>

#define SD_ChipSelect D5 // GPIO 10 is Pin 10 on Feather and D5 on Particle Boron Board

// Extern variables
extern SdFat SD;
extern File SD_fp;
extern char SD_obsdir[];
extern bool SD_exists;
extern char SD_wifi_file[];

// Function prototypes
void SD_initialize();
void SD_LogObservation(char *observations);