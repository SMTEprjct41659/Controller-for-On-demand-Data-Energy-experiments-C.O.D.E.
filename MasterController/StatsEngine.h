#ifndef STATS_ENGINE_H
#define STATS_ENGINE_H
#include <Arduino.h>

void initStats();
void resetStats();
void updateVoltage(double voltage);
void updateCurrent(double current);
void updatePower(double power);

double getVoltageMean();
double getVoltageStdDev();
double getVoltageMin();
double getVoltageMax();
double getVoltageRipple();     // percent
double getCurrentMean();
double getCurrentStdDev();
double getCurrentMin();
double getCurrentMax();
double getPowerMean();
double getPowerStdDev();
double getPowerMin();
double getPowerMax();
unsigned long getSampleCount();
String generateStatsReport();
#endif
