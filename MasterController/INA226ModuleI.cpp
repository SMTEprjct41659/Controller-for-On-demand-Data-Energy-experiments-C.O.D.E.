#include "INA226ModuleI.h"
#include "config.h"
#include <Wire.h>
#include <INA226.h>

extern unsigned long lastSwitchTime;

static INA226 ina(0x40);   // primary Wire bus, shared with BH1750
static float currentGain = 1.0f;      // fine-trim only, hardware calibration now correct
static float currentOffset = 0.0f;
static int status = 0;

bool initINA226_I() {
  Wire.begin(INA226ISDA, INA226ISCL);   // primary I2C bus, shared with BH1750
  Wire.setClock(100000);
  if (!ina.begin()) { status = -10; return false; }
  // Real shunt is R010 = 0.010 ohm, DMM-confirmed accurate at nominal value.
  // maxCurrent set to 0.5A (500mA): ~20x headroom above typical steady-state
  // readings (<100mA in practice), balancing resolution against some margin
  // for capacitor-bank charging transients. Current_LSB ~15.3uA/bit.
  if (ina.setMaxCurrentShunt(0.5f, 0.010f) != INA226_ERR_NONE) { status = -1; return false; }
  ina.setAverage(INA226_16_SAMPLES);
  ina.setBusVoltageConversionTime(INA226_1100_us);
  ina.setShuntVoltageConversionTime(INA226_1100_us);
  ina.setModeShuntBusContinuous();
  status = 0;
  return true;
}

void setCalibration_I(float gain, float offset) { currentGain = gain; currentOffset = offset; }

bool readMeasurement_I(float &v, float &c, float &p) {
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

String getINA226Status_I() { return "INA226 I status: " + String(status); }
