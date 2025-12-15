/*
 * ======================================================================================================================
 * ps.cpp - Particle Support Funtions
 * ======================================================================================================================
 */
#include <Particle.h>

#include "include/qc.h"
#include "include/output.h"
#include "include/ps.h"
#include "include/main.h"

 #if PLATFORM_ID == PLATFORM_BORON
const char* pinNames[] = {
    "A0", "A1", "A2", "A3", "A4", "A5",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8",
    "D9", "D10", "D11", "D12", "D13", "D14", "D15",
    "SDA", "SCL", "TX", "RX", "MISO", "MOSI", "SCK", "SS",
    "WKP", "VUSB", "Li+"
};
#endif

#if PLATFORM_ID == PLATFORM_ARGON
const char* pinNames[] = {
    "A0", "A1", "A2", "A3", "A4", "A5",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8",
    "D9", "D10", "D11", "D12", "D13", "D14", "D15", "D16", "D17", "D18", "D19",
    "SDA", "SCL", "TX", "RX", "MISO", "MOSI", "SCK", "SS",
    "WKP", "VUSB", "Li+", "EN", "3V3", "GND"
};
#endif

 /*
 * ======================================================================================================================
 * GetPinName() - return the pin name in provider buffer
 * ======================================================================================================================
 */
void GetPinName(pin_t pin, char *pinname) {
  struct PinMap {
    pin_t pin;
    const char *name;
  };

#if (PLATFORM_ID == PLATFORM_MSOM)
  const PinMap pinTable[] = {
    { A0, "A0" },   // Same pin as D19 
    { A1, "A1" },   // Same pin as D18 
    { A2, "A2" },   // Same pin as D17 
    { A5, "A5" },   // Same pin as D14 
    { A6, "A6" },   // Same pin as D29
    { D0, "D0" },   // Same pin as SDA 
    { D1, "D1" },   // Same pin as SCL
    { D2, "D2" }, 
    { D3, "D3" },
    { D4, "D4" }, 
    { D5, "D5" }, 
    { D6, "D6" }, 
    { D9, "D9" },   // Same pin as TX
    { D10, "D10" }, // Same pin as RX
    { D11, "D11" }, // Same pin as MISO
    { D12, "D12" }, // Same pin as MOSI
    { D13, "D13" }, // Same pin as SCK
    { D20, "D20" }, { D21, "D21" }, { D22, "D22" }, { D24, "D24" },
    { D25, "D25" }, { D26, "D26" }, { D27, "D27" }
  };
#else
  const PinMap pinTable[] = {
    { A0, "A0" },   // Same pin as D19 
    { A1, "A1" },   // Same pin as D18 
    { A2, "A2" },   // Same pin as D17 
    { A3, "A3" },   // Same pin as D16
    { A4, "A4" },   // Same pin as D15
    { A5, "A5" },   // Same pin as D14 
    { D0, "D0" },   // Same pin as SDA 
    { D1, "D1" },   // Same pin as SCL
    { D2, "D2" }, 
    { D3, "D3" },
    { D4, "D4" }, 
    { D5, "D5" }, 
    { D6, "D6" }, 
    { D7, "D7" },
    { D8, "D8" },
    { D9, "D9" },   // Same pin as TX
    { D10, "D10" }, // Same pin as RX
    { D11, "D11" }, // Same pin as MISO
    { D12, "D12" }, // Same pin as MOSI
    { D13, "D13" }, // Same pin as SCK
  };
#endif

  for (size_t i = 0; i < sizeof(pinTable) / sizeof(pinTable[0]); ++i) {
      if (pin == pinTable[i].pin) {
          strcpy(pinname, pinTable[i].name);
          return;
      }
  }

  strcpy(pinname, "NF");
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

    System.reset();

    // Never gets here and thus can never send a ack to Particle Portal
    return(0);  
  }
  else {
    Output("DoAction:UKN"); 
    return(-1);
  }
}
