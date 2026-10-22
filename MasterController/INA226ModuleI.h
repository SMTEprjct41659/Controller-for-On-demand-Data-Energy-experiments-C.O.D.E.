#ifndef INA226MODULEI_H
#define INA226MODULEI_H
#include <Arduino.h>
bool initINA226_I();
bool readMeasurement_I(float &busVoltage, float &current, float &power);
void setCalibration_I(float gain, float offset);
String getINA226Status_I();
#endif
