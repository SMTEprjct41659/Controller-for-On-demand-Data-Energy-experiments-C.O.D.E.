#include "telegramchatbot.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

const char *ssid     = "";
const char *password = "";
#define BOT_TOKEN ""
#define CHAT_ID   ""

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

portMUX_TYPE telegramMux = portMUX_INITIALIZER_UNLOCKED;
QueueHandle_t telegramOutbox = nullptr;
TaskHandle_t telegramTaskHandle = nullptr;

volatile bool startExp = false;
volatile bool stopExp = false;
volatile bool experimentRunning = false;
volatile int selectedExperiment = 0;
volatile char selectedMode[16] = {0};

static bool welcomeSent = false;
static unsigned long bootTime = 0;
unsigned long lastWiFiRetry = 0;
const unsigned long WIFI_RETRY_MS = 10000;

struct TelegramOutgoingMessage { char text[TELEGRAM_TEXT_LIMIT]; };

static void queueTelegramMessage(const String &msg) {
  if (!msg.length() || !telegramOutbox) return;
  TelegramOutgoingMessage out = {};
  msg.substring(0, TELEGRAM_TEXT_LIMIT-1).toCharArray(out.text, TELEGRAM_TEXT_LIMIT);
  xQueueSend(telegramOutbox, &out, 0);
}

static void connectWiFiIfNeeded() {
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - lastWiFiRetry < WIFI_RETRY_MS) return;
  lastWiFiRetry = millis();
  WiFi.begin(ssid, password);
}

static String mainKeyboard() {
  return "[[\"Experiment 1\",\"Experiment 2\",\"Experiment 3\",\"Experiment 4\"]]";
}
static String experimentKeyboard() {
  return "[[\"Hydro Power Mode\",\"Wind Power Mode\",\"Solar Power Mode\"],[\"🔙 Back\"]]";
}
static String stopKeyboard() { return "[[\"⏹ STOP\"]]"; }

static void sendWelcomeIfNeeded() {
  if (welcomeSent) return;
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - bootTime < 8000) return;
  bot.sendMessageWithReplyKeyboard(CHAT_ID, "Welcome! Select experiment:", "", mainKeyboard(), true);
  welcomeSent = true;
}

static void flushTelegramOutbox() {
  if (WiFi.status() != WL_CONNECTED || !telegramOutbox) return;
  TelegramOutgoingMessage out;
  while (xQueueReceive(telegramOutbox, &out, 0) == pdTRUE)
    bot.sendMessage(CHAT_ID, String(out.text), "");
}

static String currentMenu = "main";
static int activeExperiment = 0;

static void handleIncomingMessages() {
  if (WiFi.status() != WL_CONNECTED) return;
  int n = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < n; i++) {
    String text = bot.messages[i].text;
    if (text == "/start") {
      bot.sendMessageWithReplyKeyboard(CHAT_ID, "Welcome!", "", mainKeyboard(), true);
      currentMenu = "main";
      continue;
    }
    if (currentMenu == "main") {
      if (text == "Experiment 1" || text == "Experiment 2" || text == "Experiment 3" || text == "Experiment 4") {
        int expNum = text.charAt(11) - '0';
        activeExperiment = expNum;
        bot.sendMessageWithReplyKeyboard(CHAT_ID, "Experiment " + String(expNum) + " selected.\nChoose mode:", "", experimentKeyboard(), true);
        currentMenu = "experiment";
        continue;
      }
      bot.sendMessageWithReplyKeyboard(CHAT_ID, "Use buttons.", "", mainKeyboard(), true);
      continue;
    }
    if (currentMenu == "experiment") {
      if (text == "Hydro Power Mode" || text == "Wind Power Mode" || text == "Solar Power Mode") {
        String mode;
        if (text == "Hydro Power Mode") mode = "hydro";
        else if (text == "Wind Power Mode") mode = "wind";
        else mode = "solar";
        if (experimentRunning) {
          bot.sendMessage(CHAT_ID, "Experiment running. STOP first.", "");
          continue;
        }
        portENTER_CRITICAL(&telegramMux);
        startExp = true; stopExp = false;
        selectedExperiment = activeExperiment;
        strncpy((char*)selectedMode, mode.c_str(), 15);
        portEXIT_CRITICAL(&telegramMux);
        String modeName = mode; modeName[0] = toupper(modeName[0]);
        bot.sendMessageWithReplyKeyboard(CHAT_ID,
          "Experiment " + String(activeExperiment) + " – " + modeName + " mode started.\nSTOP to end.",
          "", stopKeyboard(), true);
        currentMenu = "running";
        continue;
      }
      if (text == "🔙 Back") {
        bot.sendMessageWithReplyKeyboard(CHAT_ID, "Select experiment:", "", mainKeyboard(), true);
        currentMenu = "main";
        continue;
      }
      bot.sendMessageWithReplyKeyboard(CHAT_ID, "Use buttons.", "", experimentKeyboard(), true);
      continue;
    }
    if (currentMenu == "running") {
      if (text == "⏹ STOP") {
        portENTER_CRITICAL(&telegramMux);
        stopExp = true; startExp = false; experimentRunning = false;
        portEXIT_CRITICAL(&telegramMux);
        bot.sendMessageWithReplyKeyboard(CHAT_ID, "Stopped. Select experiment:", "", mainKeyboard(), true);
        currentMenu = "main";
        continue;
      }
      bot.sendMessage(CHAT_ID, "Experiment in progress. STOP to end.", "");
      continue;
    }
  }
}

static void telegramTask(void *param) {
  for (;;) {
    connectWiFiIfNeeded();
    if (WiFi.status() == WL_CONNECTED) {
      sendWelcomeIfNeeded();
      flushTelegramOutbox();
      handleIncomingMessages();
    }
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void initTelegram() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  client.setInsecure();
  bootTime = millis();
  telegramOutbox = xQueueCreate(8, sizeof(TelegramOutgoingMessage));
  connectWiFiIfNeeded();
  xTaskCreatePinnedToCore(telegramTask, "TelegramTask", 12288, nullptr, 1, &telegramTaskHandle, 0);
}

void handleTelegram() {}

bool telegramStartExperiment() { portENTER_CRITICAL(&telegramMux); bool v=startExp; startExp=false; portEXIT_CRITICAL(&telegramMux); return v; }
bool telegramStopRequested()   { portENTER_CRITICAL(&telegramMux); bool v=stopExp; stopExp=false; portEXIT_CRITICAL(&telegramMux); return v; }
int  telegramGetSelectedExperiment() { portENTER_CRITICAL(&telegramMux); int v=selectedExperiment; portEXIT_CRITICAL(&telegramMux); return v; }
String telegramGetSelectedMode()     { portENTER_CRITICAL(&telegramMux); String s((const char*)selectedMode); portEXIT_CRITICAL(&telegramMux); return s; }
void telegramSetExperimentRunning(bool r) { portENTER_CRITICAL(&telegramMux); experimentRunning=r; portEXIT_CRITICAL(&telegramMux); }
void telegramSend(const String &msg) { queueTelegramMessage(msg); }
