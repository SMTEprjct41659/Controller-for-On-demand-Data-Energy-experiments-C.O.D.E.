#include "Experiment3.h"
#include "telegramchatbot.h"
#include "RelayController.h"
#include <math.h>

#define DS18B20_CONVERSION_MS 750

// Helper: split a long string into multiple Telegram messages
static void sendLongMessage(const String &msg) {
  const int chunkSize = TELEGRAM_TEXT_LIMIT - 100; // leave room
  int len = msg.length();
  for (int start = 0; start < len; start += chunkSize) {
    String chunk = msg.substring(start, min(start + chunkSize, len));
    telegramSend(chunk);
  }
}

Experiment3::Experiment3() : _state(E3_IDLE), _oneWire(ONEWIRE_PIN), _tempSensors(&_oneWire),
                              _tempConversionPending(false), _tempRequestTime(0) {}

void Experiment3::start(const String &mode) {
  _mode = mode; _mode.toLowerCase();
  _currentRun = 1;
  _state = E3_SETTLING;
  _phaseStart = millis();

  if (!initINA226_I() || !initINA226_O()) {
    telegramSend("INA226 I/O init failed");
    _state = E3_IDLE;
    return;
  }
  _tempSensors.begin();
  _tempSensors.setWaitForConversion(false);

  telegramSend("Experiment 3 - " + _mode + " Power Mode. 9 runs (1 hour each). "
               "Logging temperature every second.");
}

void Experiment3::stop() {
  generatorOff();
  _state = E3_IDLE;
  telegramSend("Experiment 3 stopped by user.");
}

void Experiment3::update() {
  if (_state == E3_IDLE) return;
  unsigned long now = millis();

  switch (_state) {
    case E3_SETTLING:
      if (now - _phaseStart >= EXP1_SETTLE_MS) {
        _state = E3_PRE_DELAY; _phaseStart = now;
        generatorOn();
      }
      break;

    case E3_PRE_DELAY:
      if (now - _phaseStart >= EXP1_PRE_DELAY_MS) {
        _state = E3_MEASURING; _phaseStart = _lastSample = now;
        _sampleCount = 0;
        resetStats(_statVin); resetStats(_statCin); resetStats(_statPin);
        resetStats(_statVout); resetStats(_statCout); resetStats(_statPout);
        resetStats(_statTemp0); resetStats(_statTemp1);

        // Clear sample arrays for this run
        _runs[_currentRun-1].temp0Samples.clear();
        _runs[_currentRun-1].temp1Samples.clear();

        _tempSensors.requestTemperatures();
        _tempRequestTime = now;
        _tempConversionPending = true;

        telegramSend("Run " + String(_currentRun) + " measurement started (1 hour).");
      }
      break;

    case E3_MEASURING:
      if (now - _phaseStart >= EXP3_MEASURE_MS) {
        generatorOff();

        _runs[_currentRun-1].vinMean   = getMean(_statVin);
        _runs[_currentRun-1].vinRipple = getRipple(_statVin);
        _runs[_currentRun-1].cinMean   = getMean(_statCin);
        _runs[_currentRun-1].pinMean   = getMean(_statPin);
        _runs[_currentRun-1].voutMean  = getMean(_statVout);
        _runs[_currentRun-1].voutRipple= getRipple(_statVout);
        _runs[_currentRun-1].coutMean  = getMean(_statCout);
        _runs[_currentRun-1].poutMean  = getMean(_statPout);
        _runs[_currentRun-1].temp0Mean = getMean(_statTemp0);
        _runs[_currentRun-1].temp0Min  = _statTemp0.min;
        _runs[_currentRun-1].temp0Max  = _statTemp0.max;
        _runs[_currentRun-1].temp0Ripple = getRipple(_statTemp0);
        _runs[_currentRun-1].temp1Mean = getMean(_statTemp1);
        _runs[_currentRun-1].temp1Min  = _statTemp1.min;
        _runs[_currentRun-1].temp1Max  = _statTemp1.max;
        _runs[_currentRun-1].temp1Ripple = getRipple(_statTemp1);
        _runs[_currentRun-1].efficiency = (_runs[_currentRun-1].pinMean != 0) ?
          (_runs[_currentRun-1].poutMean / _runs[_currentRun-1].pinMean) * 100.0 : 0.0;

        sendRunSummary(_currentRun);

        if (_currentRun >= EXP1_TOTAL_RUNS) {
          _state = E3_COMPLETE;
          sendFinalReport();
          _state = E3_IDLE;
          telegramSetExperimentRunning(false);
        } else {
          _state = E3_BREAK; _phaseStart = now;
          telegramSend("Break " + String(EXP1_BREAK_MS/1000) + "s.");
        }
        break;
      }

      if (now - _lastSample >= SAMPLE_INTERVAL_MS) {
        _lastSample += SAMPLE_INTERVAL_MS;

        float vIn, cIn, pIn, vOut, cOut, pOut;
        bool inOK = readMeasurement_I(vIn, cIn, pIn);
        bool outOK = readMeasurement_O(vOut, cOut, pOut);
        if (inOK) {
          updateStat(_statVin, vIn);
          updateStat(_statCin, cIn);
          updateStat(_statPin, pIn);
        }
        if (outOK) {
          updateStat(_statVout, vOut);
          updateStat(_statCout, cOut);
          updateStat(_statPout, pOut);
        }

        // Temperature reading
        if (_tempConversionPending && (now - _tempRequestTime >= DS18B20_CONVERSION_MS)) {
          float t0 = _tempSensors.getTempCByIndex(0);
          float t1 = _tempSensors.getTempCByIndex(1);
          if (t0 != DEVICE_DISCONNECTED_C) {
            updateStat(_statTemp0, t0);
            _runs[_currentRun-1].temp0Samples.push_back(t0);
          }
          if (t1 != DEVICE_DISCONNECTED_C) {
            updateStat(_statTemp1, t1);
            _runs[_currentRun-1].temp1Samples.push_back(t1);
          }
          _tempSensors.requestTemperatures();
          _tempRequestTime = now;
        }

        _sampleCount++;
        // sendProgress() removed
      }
      break;

    case E3_BREAK:
      if (now - _phaseStart >= EXP1_BREAK_MS) {
        _currentRun++;
        _state = E3_PRE_DELAY; _phaseStart = now;
        generatorOn();
      }
      break;

    default: break;
  }
}

// ----- statistics helpers (same as before) -----
void Experiment3::resetStats(Stats &s) { s.n = 0; s.sum = 0; s.sum2 = 0; s.min = 0; s.max = 0; }
void Experiment3::updateStat(Stats &s, double val) {
  s.n++; s.sum += val; s.sum2 += val*val;
  if (s.n == 1) { s.min = s.max = val; }
  else { if (val < s.min) s.min = val; if (val > s.max) s.max = val; }
}
double Experiment3::getMean(const Stats &s) { return s.n ? (s.sum / s.n) : 0.0; }
double Experiment3::getStdDev(const Stats &s) {
  if (s.n < 2) return 0.0;
  double mean = getMean(s);
  double variance = (s.sum2 / s.n) - (mean * mean);
  return variance > 0 ? sqrt(variance) : 0.0;
}
double Experiment3::getRipple(const Stats &s) {
  double mean = getMean(s);
  if (fabs(mean) < 1e-12) return 0.0;
  return (getStdDev(s) / fabs(mean)) * 100.0;
}

// ------------------------------------------------------------------
// Telegram report implementations
// ------------------------------------------------------------------

void Experiment3::sendProgress() { /* empty */ }

void Experiment3::sendRunSummary(int run) {
  if (run < 1 || run > EXP1_TOTAL_RUNS) return;
  RunData &d = _runs[run - 1];
  String msg = "=== Run " + String(run) + " Summary ===\n";
  msg += "Input  : Vmean=" + String(d.vinMean,3) + "V, Ripple=" + String(d.vinRipple,1) + "%, Imean=" + String(d.cinMean,4) + "A, Pmean=" + String(d.pinMean,4) + "W\n";
  msg += "Output : Vmean=" + String(d.voutMean,3) + "V, Ripple=" + String(d.voutRipple,1) + "%, Imean=" + String(d.coutMean,4) + "A, Pmean=" + String(d.poutMean,4) + "W\n";
  msg += "Efficiency: " + String(d.efficiency,1) + "%\n";
  msg += "Temp0: mean=" + String(d.temp0Mean,2) + "°C, min=" + String(d.temp0Min,2) + ", max=" + String(d.temp0Max,2) + ", ripple=" + String(d.temp0Ripple,1) + "%\n";
  msg += "Temp1: mean=" + String(d.temp1Mean,2) + "°C, min=" + String(d.temp1Min,2) + ", max=" + String(d.temp1Max,2) + ", ripple=" + String(d.temp1Ripple,1) + "%\n";

  // Append the full temperature sample lists (if any)
  if (d.temp0Samples.size() > 0) {
    msg += "\nTemp0 samples (C):\n";
    String list = "";
    for (size_t i = 0; i < d.temp0Samples.size(); i++) {
      list += String(d.temp0Samples[i], 2);
      if (i + 1 < d.temp0Samples.size()) list += ", ";
      if (list.length() > 2000) { // chunk to avoid overloading
        msg += list + "\n";
        list = "";
      }
    }
    if (list.length() > 0) msg += list + "\n";
  }
  if (d.temp1Samples.size() > 0) {
    msg += "Temp1 samples (C):\n";
    String list = "";
    for (size_t i = 0; i < d.temp1Samples.size(); i++) {
      list += String(d.temp1Samples[i], 2);
      if (i + 1 < d.temp1Samples.size()) list += ", ";
      if (list.length() > 2000) {
        msg += list + "\n";
        list = "";
      }
    }
    if (list.length() > 0) msg += list + "\n";
  }

  sendLongMessage(msg);
}

void Experiment3::sendFinalReport() {
  String msg = "=== Experiment 3 Final Report ===\n";
  for (int i = 0; i < EXP1_TOTAL_RUNS; i++) {
    RunData &d = _runs[i];
    msg += "Run " + String(i+1) + ": ";
    msg += "Vin=" + String(d.vinMean,2) + "V, Vout=" + String(d.voutMean,2) + "V, ";
    msg += "Pin=" + String(d.pinMean,3) + "W, Pout=" + String(d.poutMean,3) + "W, ";
    msg += "Eff=" + String(d.efficiency,1) + "%, ";
    msg += "T0mean=" + String(d.temp0Mean,1) + "°C, T1mean=" + String(d.temp1Mean,1) + "°C\n";
  }
  telegramSend(msg);
}
