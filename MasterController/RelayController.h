#ifndef RELAYCONTROLLER_H
#define RELAYCONTROLLER_H
#include <Arduino.h>
void initRelays();
void startSequence(String seq);
void stopAllRelays();
void generatorOn();
void generatorOff();
void activateDischargeRelay();
void deactivateDischargeRelay();
void activateBatteryDischargeRelay();
void deactivateBatteryDischargeRelay();
extern void markSwitchEvent();
#endif
