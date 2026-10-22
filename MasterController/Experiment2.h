#ifndef EXPERIMENT2_H
#define EXPERIMENT2_H
#include <Arduino.h>
#include "config.h"
#include "INA226ModuleI.h"
#include "StatsEngine.h"
#include "RelayController.h"

class Experiment2 {
public:
  Experiment2();
  void start(const String &mode, int branchIdx);
  void update();
  void stop();
  bool isRunning() const { return _state != E2_IDLE; }

private:
  enum State { E2_IDLE, E2_SETTLING, E2_PRE_DELAY, E2_MEASURING, E2_DISCHARGE, E2_BREAK, E2_COMPLETE };
  State _state;
  String _mode;
  int _branchIdx;
  int _currentRun;
  unsigned long _phaseStart, _lastSample;
  unsigned int _sampleCount;

  struct RunData { String statsReport; };
  RunData _runs[EXP1_TOTAL_RUNS];

  void sendProgress();
  void sendRunSummary(int run);
  void sendFinalReport();
};

#endif
