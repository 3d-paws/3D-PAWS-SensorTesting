/*
 * ======================================================================================================================
 *  main.h - Main Code Definations
 * ======================================================================================================================
 */
#include <Particle.h>

/*
 * ======================================================================================================================
 *  Loop Timers
 * ======================================================================================================================
 */
#define DELAY_NO_RTC              1000*60    // Loop delay when we have no valided RTC
#define CLOUD_CONNECTION_TIMEOUT  90         // Wait for N seconds to connect to the Cell Network

/*
 * ======================================================================================================================
 *  Relay Power Control Pin
 * ======================================================================================================================
 */
#define MAX_MSGBUF_SIZE 1024

// Extern variables
extern char versioninfo[];
extern char msgbuf[MAX_MSGBUF_SIZE];
extern char *msgp;
extern char Buffer32Bytes[32];
extern int LED_PIN;
extern bool JustPoweredOn;
extern int OBS_Interval;
extern bool TurnLedOff;
extern bool PostedResults;
extern time32_t Time_of_last_obs;
extern time32_t Time_of_next_obs;
extern uint64_t LastTimeUpdate;

#if PLATFORM_ID == PLATFORM_BORON
extern PMIC pmic; // Power Management IC (bq24195) I2C 0x6B
#endif

// Function prototypes