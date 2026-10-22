#include "INA226ModuleO.h"
#include "config.h"
#include <Wire.h>
#include <INA226.h>

extern unsigned long lastSwitchTime;

// NOTE: second constructor argument tells the library to talk over Wire1
// instead of the default Wire bus. Confirm your installed INA226 library
// version supports this (RobTillaart's INA226 does, via TwoWire* param).
static INA226 ina(0x40, &Wire1);
static float currentGain = 1.0f;      // fine-trim only, hardware calibration now correct
static float currentOffset = 0.0f;
static int status = 0;

bool initINA226_O() {
  Wire1.begin(INA226OSDA, INA226OSCL);   // second I2C bus, separate pins from Module I/BH1750
  Wire1.setClock(100000);
  if (!ina.begin()) { status = -10; return false; }
  // Real shunt is R010 = 0.010 ohm, DMM-confirmed accurate at nominal value.
  // maxCurrent set to 0.5A (500mA): 20x headroom above the real LTC3780
  // operating point (~25mA), balancing resolution against some safety margin.
  // Current_LSB ~15.3uA/bit.
  if (ina.setMaxCurrentShunt(0.5f, 0.010f) != INA226_ERR_NONE) { status = -1; return false; }
  ina.setAverage(INA226_16_SAMPLES);
  ina.setBusVoltageConversionTime(INA226_1100_us);
  ina.setShuntVoltageConversionTime(INA226_1100_us);
  ina.setModeShuntBusContinuous();
  status = 0;
  return true;
}

void setCalibration_O(float gain, float offset) { currentGain = gain; currentOffset = offset; }

bool readMeasurement_O(float &v, float &c, float &p) {
  if (millis() - lastSwitchTime < 200) return false;
  if (status != 0) return false;
  float rawV = ina.getBusVoltage();
  float rawI = ina.getCurrent();          // now correctly scaled in Amps by the library
  if (isnan(rawV) || isnan(rawI)) return false;
  // sanity range matches the 500mA headroom set above
  if (rawI < -0.05f || rawI > 0.5f) return false;
  v = rawV;
  c = (rawI * currentGain) + currentOffset;
  if (v < 0.0f || c < 0.0f) return false;
  p = v * c;
  return true;
}

String getINA226Status_O() { return "INA226 O status: " + String(status); }
