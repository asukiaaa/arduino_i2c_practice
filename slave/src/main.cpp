#include <Arduino.h>
#include <Wire.h>

#define SLAVE_ADDRESS 0x08
#define SLAVE_REGISTER_WRITE_DATA_UPPER 0x00
#define SLAVE_REGISTER_WRITE_DATA_LOWER 0x01
#define SLAVE_REGISTER_READ_DATA_UPPER 0x02
#define SLAVE_REGISTER_READ_DATA_LOWER 0x03

#define REGISTER_LEN 4

uint8_t registers[REGISTER_LEN];
uint8_t registerIndex = 0;

void updateReadData() {
  uint16_t wroteData = (uint16_t) registers[SLAVE_REGISTER_WRITE_DATA_UPPER] << 8 | registers[SLAVE_REGISTER_WRITE_DATA_LOWER];
  wroteData += 2;
  registers[SLAVE_REGISTER_READ_DATA_UPPER] = wroteData >> 8;
  registers[SLAVE_REGISTER_READ_DATA_LOWER] = wroteData & 0xff;
}

void receiveEvent(int _length) {
  uint8_t readCount = 0;
  bool wroteData = false;
  Serial.print("Receive:");
  while (Wire.available() > 0) {
    uint8_t d = Wire.read();
    Serial.print(" ");
    Serial.print(d, HEX);
    if (readCount == 0) {
      registerIndex = d;
    } else {
      if (registerIndex == SLAVE_REGISTER_WRITE_DATA_LOWER ||
          registerIndex == SLAVE_REGISTER_WRITE_DATA_UPPER) {
        wroteData = true;
        registers[registerIndex] = d;
      }
      ++registerIndex;
    }
    ++readCount;
  }
  Serial.println("");
  if (wroteData) {
    updateReadData();
  }
}

void requestEvent() {
  Wire.write(&registers[registerIndex], REGISTER_LEN - registerIndex);
}

void setup() {
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(115200);
}

void loop() {
  delay(100);
}
