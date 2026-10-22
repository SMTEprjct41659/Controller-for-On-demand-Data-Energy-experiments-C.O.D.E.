#include "config.h"
#include "telegramchatbot.h"
#include "RelayController.h"
#include "StatsEngine.h"
#include "INA226ModuleI.h"
#include "INA226ModuleO.h"
#include "Experiment1.h"
#include "Experiment2.h"
#include "Experiment3.h"

// ================= Global timing for INA226 modules =================
unsigned long lastSwitchTime = 0;

// This function is called by RelayController and INA226 modules
// every time a relay changes state, to suppress noisy readings.
void markSwitchEvent() {
  lastSwitchTime = millis();
}

// ================= Experiment objects =================
Experiment1 experiment1;
Experiment2 experiment2;
Experiment3 experiment3;

// ================= Setup =================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nMaster Controller - initialising...");

  // NOTE: no manual Wire.begin() here - each experiment's init function
  // (initINA226_I / initINA226_O) handles I2C bus setup internally when
  // that experiment actually starts.

  // Initialise all relay pins to OFF state
  initRelays();

  // Start the Telegram bot (FreeRTOS task)
  initTelegram();

  Serial.println("Master Controller ready.");
}

// ================= Main loop =================
void loop() {
  // Keep Telegram bot responsive
  handleTelegram();

  // ===== Handle new experiment start =====
  if (telegramStartExperiment()) {
    int exp = telegramGetSelectedExperiment();
    String mode = telegramGetSelectedMode();

    switch (exp) {
      case 1:
        experiment1.start(mode);
        break;
      case 2:
        // For now, we always use capacitor branch 1.
        // You can later modify the bot to ask for the branch number.
        experiment2.start(mode, 1);
        break;
      case 3:
        experiment3.start(mode);
        break;
    }
    telegramSetExperimentRunning(true);
  }

  // ===== Run active experiment state machines =====
  if (experiment1.isRunning()) experiment1.update();
  else if (experiment2.isRunning()) experiment2.update();
  else if (experiment3.isRunning()) experiment3.update();

  // ===== Handle STOP request =====
  if (telegramStopRequested()) {
    if (experiment1.isRunning()) experiment1.stop();
    else if (experiment2.isRunning()) experiment2.stop();
    else if (experiment3.isRunning()) experiment3.stop();
    telegramSetExperimentRunning(false);
  }

  // Small yield to avoid watchdog issues
  delay(50);
}
