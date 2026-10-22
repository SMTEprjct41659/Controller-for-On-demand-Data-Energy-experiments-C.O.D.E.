#include "RelayController.h"
#include "config.h"

static const int capacitorPins[5] = {RELAY_CAP1, RELAY_CAP2, RELAY_CAP3, RELAY_CAP4, RELAY_CAP5};
static const int dischargePin = RELAY_CAP_DISCHARGE;
static const int generatorPin = RELAY_GENERATOR;
static const int batteryDischargePin = RELAY_BAT_DISCHARGE;

static void setRelay(int pin, bool on) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, on ? (RELAY_ACTIVE_LOW ? LOW : HIGH) : (RELAY_ACTIVE_LOW ? HIGH : LOW));
}

void initRelays() {
  for (int i=0;i<5;i++) { pinMode(capacitorPins[i],OUTPUT); digitalWrite(capacitorPins[i], RELAY_ACTIVE_LOW ? HIGH : LOW); }
  pinMode(dischargePin, OUTPUT); digitalWrite(dischargePin, RELAY_ACTIVE_LOW ? HIGH : LOW);
  pinMode(generatorPin, OUTPUT); digitalWrite(generatorPin, RELAY_ACTIVE_LOW ? HIGH : LOW);
  pinMode(batteryDischargePin, OUTPUT); digitalWrite(batteryDischargePin, RELAY_ACTIVE_LOW ? HIGH : LOW);
}

void stopAllRelays() {
  markSwitchEvent();
  for (int i=0;i<5;i++) { setRelay(capacitorPins[i], false); delay(2); }
}

void startSequence(String seq) {
  stopAllRelays();
  if (seq=="0") return;
  for (int i=0;i<seq.length();i++) {
    int idx = seq[i]-'1';
    if (idx>=0 && idx<5) { markSwitchEvent(); setRelay(capacitorPins[idx], true); delay(5); }
  }
}

void generatorOn()  { markSwitchEvent(); setRelay(generatorPin, true); }
void generatorOff() { markSwitchEvent(); setRelay(generatorPin, false); }
void activateDischargeRelay()   { markSwitchEvent(); setRelay(dischargePin, true); }
void deactivateDischargeRelay() { markSwitchEvent(); setRelay(dischargePin, false); }
void activateBatteryDischargeRelay()   { markSwitchEvent(); setRelay(batteryDischargePin, true); }
void deactivateBatteryDischargeRelay() { markSwitchEvent(); setRelay(batteryDischargePin, false); }
