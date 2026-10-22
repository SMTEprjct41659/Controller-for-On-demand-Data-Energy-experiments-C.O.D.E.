#ifndef EXPERIMENT1_H
#define EXPERIMENT1_H
#include <Arduino.h>
#include "config.h"
#include "INA226ModuleI.h"
#include "StatsEngine.h"
#include "RelayController.h"

class Experiment1 {
public:
  Experiment1();
  void start(const String &mode);
  void update();
  void stop();
  bool isRunning() const { return _state != E1_IDLE; }

private:
  enum State { E1_IDLE, E1_SETTLING, E1_PRE_DELAY, E1_MEASURING, E1_BREAK, E1_COMPLETE };
  State _state;
  String _mode;
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
