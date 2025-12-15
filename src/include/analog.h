/*
 * ======================================================================================================================
 *  analog.h
 * ======================================================================================================================
 */
#define ANALOG_PIN_COUNT 6

typedef struct {
  bool          inuse;
  int           numReadings;
  int           delayMs;
  int           median;
  int           raw;
  float         average;
} ANALOG_PIN;

// Extern variables
extern ANALOG_PIN analog_pins[ANALOG_PIN_COUNT];

// Function prototypes
void analog_initialize();
void readAnalogPin(int pin);