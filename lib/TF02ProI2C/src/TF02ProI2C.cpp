#include "TF02ProI2C.h"

TF02ProI2C::TF02ProI2C(TwoWire& wire, uint8_t addr) : _wire(wire), _addr(addr) {}

bool TF02ProI2C::begin() {
  _wire.begin();
  _wire.beginTransmission(_addr);
  return _wire.endTransmission() == 0;
}

uint8_t TF02ProI2C::checksum(const uint8_t* data, size_t len) const {
  uint16_t sum = 0;
  for (size_t i = 0; i < len; i++) sum += data[i];
  return (uint8_t)(sum & 0xFF);
}

bool TF02ProI2C::writeBytes(const uint8_t* data, size_t len) {
  _wire.beginTransmission(_addr);
  for (size_t i = 0; i < len; i++) _wire.write(data[i]);
  return _wire.endTransmission() == 0;
}

bool TF02ProI2C::sendCommand(const uint8_t* data, size_t len) {
  return writeBytes(data, len);
}

bool TF02ProI2C::requestFrameBytes(uint8_t* buf, size_t len, uint16_t timeoutMs) {
  uint32_t start = millis();
  while ((uint32_t)(millis() - start) < timeoutMs) {
    uint8_t n = _wire.requestFrom((int)_addr, (int)len);
    if (n == len) {
      for (size_t i = 0; i < len; i++) buf[i] = _wire.read();
      return true;
    }
    while (_wire.available()) _wire.read();
    delay(1);
  }
  return false;
}

bool TF02ProI2C::readFrame(uint16_t &distanceCm, uint16_t &strength, float &temperatureC, uint16_t timeoutMs) {
  const uint8_t readCmd[] = {0x5A, 0x05, 0x00, 0x01, 0x60};
  if (!sendCommand(readCmd, sizeof(readCmd))) return false;

  uint8_t buf[9];
  if (!requestFrameBytes(buf, sizeof(buf), timeoutMs)) return false;

  if (buf[0] != 0x59 || buf[1] != 0x59) return false;
  if (checksum(buf, 8) != buf[8]) return false;

  distanceCm = (uint16_t)buf[2] | ((uint16_t)buf[3] << 8);
  strength   = (uint16_t)buf[4] | ((uint16_t)buf[5] << 8);
  uint16_t rawTemp = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);
  temperatureC = rawTemp / 8.0f - 256.0f;
  return true;
}

bool TF02ProI2C::setI2CMode() {
  const uint8_t cmd[] = {0x5A, 0x05, 0x0A, 0x01, 0x66};
  return sendCommand(cmd, sizeof(cmd));
}

bool TF02ProI2C::setAddress(uint8_t newAddr) {
  uint8_t cmd[] = {0x5A, 0x05, 0x0B, newAddr, 0x00};
  cmd[4] = checksum(cmd, 4);
  return sendCommand(cmd, sizeof(cmd));
}

bool TF02ProI2C::saveSettings() {
  const uint8_t cmd[] = {0x5A, 0x04, 0x11, 0x6F};
  return sendCommand(cmd, sizeof(cmd));
}
