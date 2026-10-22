/*
  I2C_Scanner.ino - dual bus version
  Scans BOTH I2C buses used in this project:
    Wire  (GPIO21/22) - INA226 Module I (0x40) + BH1750 (0x23)
    Wire1 (GPIO16/17) - INA226 Module O (0x40, alone on its own bus)

  Expected result once everything is wired correctly:
    Bus 1 (Wire):  0x23 (BH1750), 0x40 (Module I)
    Bus 2 (Wire1): 0x40 (Module O)
*/
#include <Wire.h>

#define BUS1_SDA 21
#define BUS1_SCL 22
#define BUS2_SDA 16
#define BUS2_SCL 17

void scanBus(TwoWire &bus, const char* label) {
  Serial.print("Scanning ");
  Serial.print(label);
  Serial.println(" ...");
  int nDevices = 0;
  for (byte address = 1; address < 127; address++) {
    bus.beginTransmission(address);
    byte error = bus.endTransmission();
    if (error == 0) {
      Serial.print("  Device at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
  }
  if (!nDevices) Serial.println("  No I2C devices found on this bus");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Wire.begin(BUS1_SDA, BUS1_SCL);
  Wire1.begin(BUS2_SDA, BUS2_SCL);
  Serial.println("Dual-bus I2C Scanner ready.");
}

void loop() {
  scanBus(Wire, "Bus 1 (GPIO21/22)");
  scanBus(Wire1, "Bus 2 (GPIO16/17)");
  Serial.println("----------------------------------------");
  delay(5000);
}
