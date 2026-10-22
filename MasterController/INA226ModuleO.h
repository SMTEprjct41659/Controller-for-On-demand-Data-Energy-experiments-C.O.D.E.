#ifndef INA226MODULEO_H
#define INA226MODULEO_H
#include <Arduino.h>
bool initINA226_O();
bool readMeasurement_O(float &busVoltage, float &current, float &power);
void setCalibration_O(float gain, float offset);
String getINA226Status_O();
#endif
