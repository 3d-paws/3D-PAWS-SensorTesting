/*
 * ======================================================================================================================
 *  Analog.h
 * ======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * Analog Pin Monitoring - Adjust the array to control what analog pins are being monitored
 *=======================================================================================================================
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

ANALOG_PIN analog_pins[ANALOG_PIN_COUNT];

/* 
 *=======================================================================================================================
 * analog_initialize()
 *=======================================================================================================================
 */
void analog_initialize() {

  // Pin A0
  analog_pins[0].inuse = true;
  analog_pins[0].numReadings = 3;
  analog_pins[0].delayMs = 200;

  // Pin A1
  analog_pins[1].inuse = false;
  analog_pins[1].numReadings = 3;
  analog_pins[1].delayMs = 200;

  // Pin A2
  analog_pins[2].inuse = false;
  analog_pins[2].numReadings = 3;
  analog_pins[2].delayMs = 200;

  // Pin A3
  analog_pins[3].inuse = false;
  analog_pins[3].numReadings = 3;
  analog_pins[3].delayMs = 200;

  // Pin A4
  analog_pins[4].inuse = false;
  analog_pins[4].numReadings = 3;
  analog_pins[4].delayMs = 200;

  // Pin A5
  analog_pins[5].inuse = false;
  analog_pins[5].numReadings = 3;
  analog_pins[5].delayMs = 200;

  // Initialize obs values to 0
  for (int pin=0; pin<ANALOG_PIN_COUNT; pin++) {
    analog_pins[pin].median = 0;
    analog_pins[pin].raw = 0;
    analog_pins[pin].average = 0.0;
    if (analog_pins[pin].inuse) {
      sprintf (Buffer32Bytes, "Pin A%d: Enabled", pin);
      Output(Buffer32Bytes);
    }
  }
}

/* 
 *=======================================================================================================================
 * readAnalogPin()
 *=======================================================================================================================
 */
void readAnalogPin(int pin) {
  unsigned int readings[analog_pins[pin].numReadings];
  float total     = 0.0;

  // Perform readings
  for (int i = 0; i < analog_pins[pin].numReadings; i++) {
    switch (pin) {
      case 0 : readings[i] = analogRead(A0); break;
      case 1 : readings[i] = analogRead(A1); break;
      case 2 : readings[i] = analogRead(A2); break;
      case 3 : readings[i] = analogRead(A3); break;
      case 4 : readings[i] = analogRead(A4); break;
      case 5 : readings[i] = analogRead(A5); break;
      default: break;
    }

    sprintf (Buffer32Bytes, "Pin A%d %d", pin, readings[i]);
    Output(Buffer32Bytes);

    total += readings[i];
    delay(analog_pins[pin].delayMs);
  }

  analog_pins[pin].raw = analogRead(A0 + pin);

  // Calculate average
  analog_pins[pin].average = total / analog_pins[pin].numReadings;

  // Sort the array to find median
  mysort(readings, analog_pins[pin].numReadings);

  // Median is the middle value
  analog_pins[pin].median = readings[analog_pins[pin].numReadings / 2];
}