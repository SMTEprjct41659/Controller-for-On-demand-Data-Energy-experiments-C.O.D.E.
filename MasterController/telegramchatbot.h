#ifndef TELEGRAMCHATBOT_H
#define TELEGRAMCHATBOT_H
#include <Arduino.h>
#define TELEGRAM_TEXT_LIMIT 3000

void initTelegram();
void handleTelegram();
bool telegramStartExperiment();
bool telegramStopRequested();
void telegramSetExperimentRunning(bool running);
int  telegramGetSelectedExperiment();
String telegramGetSelectedMode();
void telegramSend(const String &msg);
#endif
