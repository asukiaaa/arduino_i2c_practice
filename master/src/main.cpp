#include <Arduino.h>
#include <Wire.h>

#define SLAVE_ADDRESS 0x08
#define SLAVE_REGISTER_WRITE_DATA_UPPER 0x00
#define SLAVE_REGISTER_WRITE_DATA_LOWER 0x01
#define SLAVE_REGISTER_READ_DATA_UPPER 0x02
#define SLAVE_REGISTER_READ_DATA_LOWER 0x03

uint8_t writeToDevice(TwoWire &wire, uint8_t deviceAddress, uint8_t *data, uint8_t length, bool stop=true) {
  wire.beginTransmission(deviceAddress);
  wire.write(data, length);
  return wire.endTransmission(stop);
}

uint8_t writeToRegister(TwoWire &wire, uint8_t deviceAddress, uint8_t registerAddress, uint8_t *data, uint8_t length, bool stop=true) {
  wire.beginTransmission(deviceAddress);
  wire.write(registerAddress);
  wire.write(data, length);
  return wire.endTransmission(stop);
}

uint8_t readFromDevice(TwoWire &wire, uint8_t deviceAddress, uint8_t *data, uint8_t length, bool stop=true) {
  const uint8_t resultLen = wire.requestFrom(deviceAddress, length, stop);
  uint8_t dataIndex = 0;
  while (wire.available()) {
    char d = wire.read();
    if (dataIndex < length) {
      data[dataIndex] = d;
      ++dataIndex;
    }
  }
  return resultLen;
}

uint8_t readFromRegister(TwoWire &wire, uint8_t deviceAddress, uint8_t registerAddress, uint8_t *data, uint8_t length, bool stop=true) {
  uint8_t result = writeToDevice(wire, deviceAddress, &registerAddress, 1, false);
  if (result != 0) {
    return 0;
  }
  return readFromDevice(wire, deviceAddress, data, length, stop);
}

void writeAndPrintResult(uint16_t value) {
  uint8_t data[2];
  data[0] = value >> 8;
  data[1] = value & 0xff;
  if (writeToRegister(Wire, SLAVE_ADDRESS, SLAVE_REGISTER_WRITE_DATA_UPPER, data, 2) == 0) {
    Serial.print("Wrote data: ");
    Serial.print(value);
    Serial.print(" ( ");
    Serial.print(data[0], HEX);
    Serial.print(" ");
    Serial.print(data[1], HEX);
    Serial.println(" )");
  } else {
    Serial.println("Failed to write data");
  }
}

void readAndPrintResult() {
  uint8_t data[2];
  uint8_t readLen = readFromRegister(Wire, SLAVE_ADDRESS, SLAVE_REGISTER_READ_DATA_UPPER, data, 2);
  if (readLen != 0) {
    uint16_t value = 0;
    if (readLen >= 2) {
      value = (uint16_t)data[0] << 8 | data[1];
    }
    Serial.print("Read data: " + String(value) + " (");
    for (uint8_t i=0; i<readLen; ++i) {
      Serial.print(" ");
      Serial.print(data[i], HEX);
    }
    Serial.println(" )");
  } else {
    Serial.println("Failed to read data");
  }
}

void setup() {
  Wire.begin();
  Serial.begin(115200);
}

void loop() {
  writeAndPrintResult(1000);
  readAndPrintResult(); // expects 1002
  writeAndPrintResult(0xff);
  readAndPrintResult(); // expects 257 (0x0101)
  Serial.println("at " + String(millis()));
  delay(1000);
}
