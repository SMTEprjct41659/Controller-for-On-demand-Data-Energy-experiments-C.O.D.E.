#include "Experiment2.h"
#include "telegramchatbot.h"
#include "RelayController.h"

Experiment2::Experiment2() : _state(E2_IDLE), _branchIdx(1) {}

void Experiment2::start(const String &mode, int branchIdx) {
  _mode = mode; _mode.toLowerCase();
  _branchIdx = constrain(branchIdx, 1, 5);
  _currentRun = 1;
  _state = E2_SETTLING;
  _phaseStart = millis();

  if (!initINA226_I()) {
    telegramSend("INA226 I init failed");
    _state = E2_IDLE;
    return;
  }

  startSequence(String(_branchIdx));

  telegramSend("Experiment 2 - " + _mode + " Power Mode, branch " + String(_branchIdx) +
               ". 9 runs (5 min each).");
}

void Experiment2::stop() {
  generatorOff();
  stopAllRelays();
  _state = E2_IDLE;
  telegramSend("Experiment 2 stopped by user.");
}

void Experiment2::update() {
  if (_state == E2_IDLE) return;
  unsigned long now = millis();

  switch (_state) {
    case E2_SETTLING:
      if (now - _phaseStart >= EXP1_SETTLE_MS) {
        _state = E2_PRE_DELAY;
        _phaseStart = now;
        generatorOn();
      }
      break;

    case E2_PRE_DELAY:
      if (now - _phaseStart >= EXP1_PRE_DELAY_MS) {
        _state = E2_MEASURING;
        _phaseStart = _lastSample = now;
        _sampleCount = 0;
        resetStats();
        telegramSend("Run " + String(_currentRun) + " measurement started.");
      }
      break;

    case E2_MEASURING:
      if (now - _phaseStart >= EXP1_MEASURE_MS) {
        generatorOff();
        activateDischargeRelay();
        _state = E2_DISCHARGE;
        _phaseStart = now;

        String report = generateStatsReport();
        _runs[_currentRun - 1].statsReport = report;
        sendRunSummary(_currentRun);
        break;
      }

      if (now - _lastSample >= SAMPLE_INTERVAL_MS) {
        _lastSample += SAMPLE_INTERVAL_MS;
        float v, c, p;
        if (readMeasurement_I(v, c, p)) {
          updateVoltage(v); updateCurrent(c); updatePower(p);
          _sampleCount++;
        }
        // sendProgress() removed
      }
      break;

    case E2_DISCHARGE:
      if (now - _phaseStart >= 10000) {
        deactivateDischargeRelay();

        if (_currentRun >= EXP1_TOTAL_RUNS) {
          _state = E2_COMPLETE;
          sendFinalReport();
          _state = E2_IDLE;
          telegramSetExperimentRunning(false);
        } else {
          _state = E2_BREAK;
          _phaseStart = now;
          telegramSend("Break 10s before next run.");
        }
      }
      break;

    case E2_BREAK:
      if (now - _phaseStart >= EXP1_BREAK_MS) {
        _currentRun++;
        startSequence(String(_branchIdx));
        _state = E2_PRE_DELAY;
        _phaseStart = now;
        generatorOn();
      }
      break;

    default: break;
  }
}

// ------------------------------------------------------------------
// Telegram report implementations
// ------------------------------------------------------------------

void Experiment2::sendProgress() { /* empty */ }

void Experiment2::sendRunSummary(int run) {
  if (run < 1 || run > EXP1_TOTAL_RUNS) return;
  RunData &d = _runs[run - 1];
  String msg = "=== Run " + String(run) + " Summary ===\n";
  msg += d.statsReport;
  telegramSend(msg);
}

void Experiment2::sendFinalReport() {
  String msg = "=== Experiment 2 Final Report ===\n";
  for (int i = 0; i < EXP1_TOTAL_RUNS; i++) {
    msg += "Run " + String(i + 1) + ":\n";
    msg += _runs[i].statsReport;
    msg += "\n";
  }
  telegramSend(msg);
}
