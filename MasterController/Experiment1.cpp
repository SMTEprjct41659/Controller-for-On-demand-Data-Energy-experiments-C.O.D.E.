#include "Experiment1.h"
#include "telegramchatbot.h"
#include "RelayController.h"

Experiment1::Experiment1() : _state(E1_IDLE) {}

void Experiment1::start(const String &mode) {
  _mode = mode; _mode.toLowerCase();
  _currentRun = 1;
  _state = E1_SETTLING;
  _phaseStart = millis();

  if (!initINA226_I()) {
    telegramSend("INA226 I init failed");
    _state = E1_IDLE;
    return;
  }

  telegramSend("Experiment 1 - " + _mode + " Power Mode started. 9 runs (5 min each).");
}

void Experiment1::stop() {
  generatorOff();
  _state = E1_IDLE;
  telegramSend("Experiment 1 stopped by user.");
}

void Experiment1::update() {
  if (_state == E1_IDLE) return;
  unsigned long now = millis();

  switch (_state) {
    case E1_SETTLING:
      if (now - _phaseStart >= EXP1_SETTLE_MS) {
        _state = E1_PRE_DELAY;
        _phaseStart = now;
        generatorOn();
      }
      break;

    case E1_PRE_DELAY:
      if (now - _phaseStart >= EXP1_PRE_DELAY_MS) {
        _state = E1_MEASURING;
        _phaseStart = _lastSample = now;
        _sampleCount = 0;
        resetStats();
        telegramSend("Run " + String(_currentRun) + " measurement started.");
      }
      break;

    case E1_MEASURING: {
      if (now - _phaseStart >= EXP1_MEASURE_MS) {
        generatorOff();
        String report = generateStatsReport();
        _runs[_currentRun-1].statsReport = report;
        sendRunSummary(_currentRun);

        if (_currentRun >= EXP1_TOTAL_RUNS) {
          _state = E1_COMPLETE;
          sendFinalReport();
          _state = E1_IDLE;
          telegramSetExperimentRunning(false);
        } else {
          _state = E1_BREAK;
          _phaseStart = now;
          telegramSend("Break " + String(EXP1_BREAK_MS/1000) + "s.");
        }
        break;
      }

      if (now - _lastSample >= SAMPLE_INTERVAL_MS) {
        _lastSample += SAMPLE_INTERVAL_MS;
        float v, c, p;
        if (readMeasurement_I(v, c, p)) {
          updateVoltage(v);
          updateCurrent(c);
          updatePower(p);
          _sampleCount++;
        }
        // sendProgress() removed as requested
      }
      break;
    }

    case E1_BREAK:
      if (now - _phaseStart >= EXP1_BREAK_MS) {
        _currentRun++;
        _state = E1_PRE_DELAY;
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

void Experiment1::sendProgress() { /* intentionally empty */ }

void Experiment1::sendRunSummary(int run) {
  if (run < 1 || run > EXP1_TOTAL_RUNS) return;
  RunData &d = _runs[run - 1];
  String msg = "=== Run " + String(run) + " Summary ===\n";
  msg += d.statsReport;
  telegramSend(msg);
}

void Experiment1::sendFinalReport() {
  String msg = "=== Experiment 1 Final Report ===\n";
  for (int i = 0; i < EXP1_TOTAL_RUNS; i++) {
    msg += "Run " + String(i + 1) + ":\n";
    msg += _runs[i].statsReport;
    msg += "\n";
  }
  telegramSend(msg);
}
