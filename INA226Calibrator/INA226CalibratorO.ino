/*
  INA226CalibratorO.ino
  Calibrates INA226 Module O (bus: Wire1, GPIO16/17, address 0x40 - own bus) against a DMM.
  Same raw-register approach as Module I, just targeting the second I2C bus.

  Wiring reminder: Module O SDA->GPIO16, SCL->GPIO17, with 4.7k pull-ups to 3.3V
  if the board doesn't already have onboard pull-ups (check with the I2C scanner first).

  Usage: same as INA226CalibratorI.ino - watch live readings, compare to DMM,
  type the DMM's mA value + Enter to calibrate.
*/
#include <Wire.h>

#define INA226_ADDR       0x40
#define REG_CONFIG        0x00
#define REG_SHUNTVOLTAGE  0x01
#define REG_BUSVOLTAGE    0x02
#define REG_POWER         0x03
#define REG_CURRENT       0x04
#define REG_CALIBRATION   0x05

const float R_SHUNT = 0.010;
float currentLSB = 0.00005;      // 50 uA/bit
uint16_t calValue = 10240;       // starting point, matches maxCurrent=1A design

void writeRegister(uint8_t reg, uint16_t value) {
  Wire1.beginTransmission(INA226_ADDR);
  Wire1.write(reg);
  Wire1.write((value >> 8) & 0xFF);
  Wire1.write(value & 0xFF);
  Wire1.endTransmission();
}

int16_t readRegisterSigned(uint8_t reg) {
  Wire1.beginTransmission(INA226_ADDR);
  Wire1.write(reg);
  Wire1.endTransmission(false);
  Wire1.requestFrom(INA226_ADDR, 2);
  uint16_t raw = (Wire1.read() << 8) | Wire1.read();
  return (int16_t)raw;
}

void applyCalibration() {
  writeRegister(REG_CONFIG, 0x4127);
  writeRegister(REG_CALIBRATION, calValue);
}

void printReadings() {
  int16_t rawShunt = readRegisterSigned(REG_SHUNTVOLTAGE);
  int16_t rawBus   = readRegisterSigned(REG_BUSVOLTAGE);
  int16_t rawCurr  = readRegisterSigned(REG_CURRENT);
  int16_t rawPower = readRegisterSigned(REG_POWER);

  float shuntVoltage_mV = rawShunt * 0.0025;
  float busVoltage_V    = rawBus * 0.00125;
  float current_mA      = rawCurr * (currentLSB * 1000.0);
  float power_mW        = rawPower * (currentLSB * 1000.0 * 25.0);

  Serial.println(F("----------------------------------------"));
  Serial.print(F("Shunt Voltage : ")); Serial.print(shuntVoltage_mV, 4); Serial.println(F(" mV"));
  Serial.print(F("Bus Voltage   : ")); Serial.print(busVoltage_V, 4);   Serial.println(F(" V"));
  Serial.print(F("Current       : ")); Serial.print(current_mA, 2);    Serial.println(F(" mA"));
  Serial.print(F("Power         : ")); Serial.print(power_mW, 2);      Serial.println(F(" mW"));
  Serial.print(F("Current CAL reg value: ")); Serial.println(calValue);
  Serial.println(F("----------------------------------------"));
  Serial.println(F("Type DMM current in mA and press Enter to calibrate,"));
  Serial.println(F("or just wait to keep watching live readings."));
}

void checkSerialInput() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() == 0) return;

    float dmm_mA = input.toFloat();
    if (dmm_mA <= 0) {
      Serial.println(F("Invalid input. Enter a positive number in mA."));
      return;
    }

    int16_t rawCurr = readRegisterSigned(REG_CURRENT);
    float ina_mA = rawCurr * (currentLSB * 1000.0);

    if (ina_mA <= 0) {
      Serial.println(F("INA226 current reading is zero or negative, check wiring/load."));
      return;
    }

    float ratio = dmm_mA / ina_mA;
    uint16_t newCal = (uint16_t)round(calValue * ratio);

    Serial.println(F("=== Calibration update ==="));
    Serial.print(F("DMM reading      : ")); Serial.print(dmm_mA, 2); Serial.println(F(" mA"));
    Serial.print(F("INA226 reading   : ")); Serial.print(ina_mA, 2); Serial.println(F(" mA"));
    Serial.print(F("Error before     : ")); Serial.print(((ina_mA - dmm_mA) / dmm_mA) * 100.0, 3); Serial.println(F(" %"));
    Serial.print(F("Old CAL          : ")); Serial.println(calValue);
    Serial.print(F("New CAL          : ")); Serial.println(newCal);

    calValue = newCal;
    applyCalibration();

    delay(200);
    int16_t rawCurr2 = readRegisterSigned(REG_CURRENT);
    float ina_mA2 = rawCurr2 * (currentLSB * 1000.0);
    Serial.print(F("INA226 after correction: ")); Serial.print(ina_mA2, 2); Serial.println(F(" mA"));
    Serial.print(F("Error after      : ")); Serial.print(((ina_mA2 - dmm_mA) / dmm_mA) * 100.0, 3); Serial.println(F(" %"));
    Serial.println(F("=========================="));
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Wire1.begin(16, 17);   // Bus 2: Module O, alone
  applyCalibration();
  Serial.println(F("INA226 Module O Calibrator ready (bus: GPIO16/17)."));
}

unsigned long lastPrint = 0;

void loop() {
  checkSerialInput();
  if (millis() - lastPrint > 1000) {
    printReadings();
    lastPrint = millis();
  }
}
