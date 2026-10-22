#include "StatsEngine.h"
#include <math.h>

struct Stats { unsigned long samples; double mean, M2, min, max; };

static Stats vStats, cStats, pStats;
static unsigned long startTimeUs = 0, lastSampleTimeUs = 0, sampleCount = 0;

static void resetStat(Stats &s) { s.samples=0; s.mean=0; s.M2=0; s.min=0; s.max=0; }

static void updateStat(Stats &s, double val) {
  s.samples++;
  double delta = val - s.mean;
  s.mean += delta / s.samples;
  double delta2 = val - s.mean;
  s.M2 += delta * delta2;
  if (s.samples == 1) { s.min = s.max = val; }
  else {
    if (val < s.min) s.min = val;
    if (val > s.max) s.max = val;
  }
}

void initStats() { resetStats(); }

void resetStats() {
  resetStat(vStats); resetStat(cStats); resetStat(pStats);
  sampleCount = 0;
  startTimeUs = micros();
  lastSampleTimeUs = startTimeUs;
}

void updateVoltage(double v) { updateStat(vStats, v); sampleCount++; lastSampleTimeUs = micros(); }
void updateCurrent(double c) { updateStat(cStats, c); }
void updatePower(double p)    { updateStat(pStats, p); }

double getVoltageMean()  { return vStats.mean; }
double getVoltageStdDev() { if (vStats.samples<2) return 0.0; return sqrt(vStats.M2 / (double)vStats.samples); }
double getVoltageMin()   { return vStats.min; }
double getVoltageMax()   { return vStats.max; }
double getVoltageRipple() { double sd = getVoltageStdDev(); if (fabs(vStats.mean)<1e-12) return 0; return (sd / fabs(vStats.mean)) * 100.0; }

double getCurrentMean()  { return cStats.mean; }
double getCurrentStdDev(){ if (cStats.samples<2) return 0.0; return sqrt(cStats.M2 / (double)cStats.samples); }
double getCurrentMin()   { return cStats.min; }
double getCurrentMax()   { return cStats.max; }

double getPowerMean()    { return pStats.mean; }
double getPowerStdDev()  { if (pStats.samples<2) return 0.0; return sqrt(pStats.M2 / (double)pStats.samples); }
double getPowerMin()     { return pStats.min; }
double getPowerMax()     { return pStats.max; }

unsigned long getSampleCount() { return sampleCount; }

String generateStatsReport() {
  if (sampleCount < 2) return "Not enough data.";
  double duration = (double)(micros() - startTimeUs) / 1e6;
  double rate = (duration > 0) ? (double)sampleCount / duration : 0.0;
  String msg;
  msg += "Samples: " + String(sampleCount) + " Duration: " + String(duration,1) + "s Rate: " + String(rate,1) + "Hz\n";
  msg += "--- Voltage ---\nMean: " + String(getVoltageMean(),4) + "V  SD: " + String(getVoltageStdDev(),4) + "V  Ripple: " + String(getVoltageRipple(),1) + "%\nMin/Max: " + String(getVoltageMin(),4) + " / " + String(getVoltageMax(),4) + "V\n";
  msg += "--- Current ---\nMean: " + String(getCurrentMean(),4) + "A  SD: " + String(getCurrentStdDev(),4) + "A\nMin/Max: " + String(getCurrentMin(),4) + " / " + String(getCurrentMax(),4) + "A\n";
  msg += "--- Power ---\nMean: " + String(getPowerMean(),4) + "W  SD: " + String(getPowerStdDev(),4) + "W\nMin/Max: " + String(getPowerMin(),4) + " / " + String(getPowerMax(),4) + "W\n";
  return msg;
}
