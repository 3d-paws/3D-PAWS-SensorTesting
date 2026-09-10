#pragma once
#include <Arduino.h>
#include <Wire.h>

class TF02ProI2C {
public:
  explicit TF02ProI2C(TwoWire& wire = Wire, uint8_t addr = 0x10);

  bool begin();
  bool readFrame(uint16_t &distanceCm, uint16_t &strength, float &temperatureC, uint16_t timeoutMs = 100);

  bool setI2CMode();
  bool setAddress(uint8_t newAddr);
  bool saveSettings();

  uint8_t address() const { return _addr; }

private:
  TwoWire& _wire;
  uint8_t _addr;

  uint8_t checksum(const uint8_t* data, size_t len) const;
  bool writeBytes(const uint8_t* data, size_t len);
  bool sendCommand(const uint8_t* data, size_t len);
  bool requestFrameBytes(uint8_t* buf, size_t len, uint16_t timeoutMs);
};
