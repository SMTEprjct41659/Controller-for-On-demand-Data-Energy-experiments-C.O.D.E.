#ifndef EXPERIMENT3_H
#define EXPERIMENT3_H
#include <Arduino.h>
#include "config.h"
#include "INA226ModuleI.h"
#include "INA226ModuleO.h"
#include "RelayController.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <vector>   // required for storing temperature sample lists

class Experiment3 {
public:
  Experiment3();
  void start(const String &mode);
  void update();
  void stop();
  bool isRunning() const { return _state != E3_IDLE; }

private:
  enum State { E3_IDLE, E3_SETTLING, E3_PRE_DELAY, E3_MEASURING, E3_BREAK, E3_COMPLETE };
  State _state;
  String _mode;
  int _currentRun;
  unsigned long _phaseStart, _lastSample;
  unsigned int _sampleCount;

  struct Stats { unsigned long n; double sum, sum2, min, max; };
  Stats _statVin, _statCin, _statPin;
  Stats _statVout, _statCout, _statPout;
  Stats _statTemp0, _statTemp1;

  OneWire _oneWire;
  DallasTemperature _tempSensors;

  bool _tempConversionPending;
  unsigned long _tempRequestTime;

  void resetStats(Stats &s);
  void updateStat(Stats &s, double val);
  double getMean(const Stats &s);
  double getStdDev(const Stats &s);
  double getRipple(const Stats &s);

  void sendProgress();
  void sendRunSummary(int run);
  void sendFinalReport();

  struct RunData {
    double vinMean, vinRipple, cinMean, pinMean;
    double voutMean, voutRipple, coutMean, poutMean;
    double temp0Mean, temp0Min, temp0Max, temp0Ripple;
    double temp1Mean, temp1Min, temp1Max, temp1Ripple;
    double efficiency;
    std::vector<float> temp0Samples;   // collect all 3600 temperature readings
    std::vector<float> temp1Samples;   // collect all 3600 temperature readings
  };
  RunData _runs[EXP1_TOTAL_RUNS];
};

#endif
