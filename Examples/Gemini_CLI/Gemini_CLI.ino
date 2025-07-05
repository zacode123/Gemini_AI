/*

MIT License

Copyright (c) 2025 zacode123

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

*/

#define EEPROM_SIZE 1024
#define EEPROM_MAGIC 0x42

#include <EEPROM.h>
#include <Gemini_AI.h>

// ADC for Vcc measurement
ADC_MODE(ADC_VCC);

unsigned long bootTime;
String lastRequest;
String lastResponse;

// Use dynamic char* variables
char* ssid = nullptr;
char* password = nullptr;
char* model = nullptr;
char* token = nullptr;
char* systemInstruction = nullptr;

// Other config variables
int maxTokens = 1000;
float temperature = 0.8;
float TopP = 1.0;
float TopK = 40;
bool codeExecution = false;
bool googleSearch = false;
bool ledmode = true;

// Gemini instance (will be constructed later after strdup)
Gemini_AI* gemini = nullptr;

void setup() {
  Serial.begin(115200);

  // initialize default strings dynamically
  ssid = strdup("YOUR_SSID");
  password = strdup("YOUR_PASSWORD");
  model = strdup("gemini-2.0-flash");
  token = strdup("YOUR_API_KEY");
  systemInstruction = strdup("You are an highly intelligent AI assistant. You can perform various tasks with ease. You should give answer carefully without any mistake! and also remember you have 1000 tokens you have to give full response within 1000 tokens or less, but don't exceed it! Also don't tell the user that you cant use "*"! You should use emojis in response and symbols in answers that is about math, science and code etc");

  // build Gemini
  gemini = new Gemini_AI(ssid, password, model, token, maxTokens, systemInstruction, temperature, TopP, TopK, codeExecution, googleSearch, ledmode);

  if (gemini->connectToWiFi()) {
    clear();
    Serial.println("Connected to ");
    Serial.println(ssid);
    initEEPROM();
    delay(100);
    loadFromEEPROM();
    Serial.println("CLI Initialised");
    bootTime = millis();
  }
}

void printSystemInfo() {
  Serial.println(F("\033[36m\n==================[ SYSTEM INFO ]==================\033[0m\n"));

  Serial.printf("\033[33mFree Heap             : \033[32m%u bytes\033[0m\n", ESP.getFreeHeap());
  Serial.printf("\033[33mHeap Fragmentation    : \033[32m%u%%\033[0m\n", ESP.getHeapFragmentation());
  Serial.printf("\033[33mMax Free Block Size   : \033[32m%u bytes\033[0m\n", ESP.getMaxFreeBlockSize());
  Serial.printf("\033[33mFlash Chip Size       : \033[32m%u KB\033[0m\n", ESP.getFlashChipRealSize() / 1024);
  Serial.printf("\033[33mFlash Chip Speed      : \033[32m%u MHz\033[0m\n", ESP.getFlashChipSpeed() / 1000000);
  Serial.printf("\033[33mSketch Size           : \033[32m%u KB\033[0m\n", ESP.getSketchSize() / 1024);
  Serial.printf("\033[33mFree Sketch Space     : \033[32m%u KB\033[0m\n", ESP.getFreeSketchSpace() / 1024);
  Serial.printf("\033[33mChip ID               : \033[32m%u\033[0m\n", ESP.getChipId());
  Serial.printf("\033[33mCore Version          : \033[32m%s\033[0m\n", ESP.getCoreVersion());
  Serial.printf("\033[33mSDK Version           : \033[32m%s\033[0m\n", ESP.getSdkVersion());
  Serial.printf("\033[33mCPU Frequency         : \033[32m%u MHz\033[0m\n", ESP.getCpuFreqMHz());
  Serial.printf("\033[33mBoot Version          : \033[32m%u\033[0m\n", system_get_boot_version());
  Serial.printf("\033[33mBoot Mode             : \033[32m%u\033[0m\n", ESP.getBootMode());
  Serial.printf("\033[33mReset Info            : \033[32m%s\033[0m\n", ESP.getResetInfo().c_str());
  Serial.printf("\033[33mReset Reason          : \033[32m%s\033[0m\n", ESP.getResetReason().c_str());
  Serial.printf("\033[33mHostname              : \033[32m%s\033[0m\n", WiFi.hostname().c_str());
  Serial.printf("\033[33mMAC Address           : \033[32m%s\033[0m\n", WiFi.macAddress().c_str());
  Serial.printf("\033[33mIP Address            : \033[32m%s\033[0m\n", WiFi.localIP().toString().c_str());
  Serial.printf("\033[33mSignal Strength       : \033[32m%d dBm\033[0m\n", WiFi.RSSI());

  unsigned long elapsed = millis() - bootTime;
  unsigned long totalSeconds = elapsed / 1000;
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;

  Serial.printf("\033[33mUptime                : \033[32m%02lu:%02lu:%02lu\033[0m\n", hours, minutes, seconds);
  Serial.printf("\033[33mVoltage (ADC read)    : \033[32m%.2f V\033[0m\n", ESP.getVcc() / 1024.0);

  Serial.println(F("\033[36m\n===================================================\033[0m\n"));
}

void showCredentials() {
  Serial.println(F("\033[36m\n================[ GEMINI AI CONFIG ]================\033[0m\n"));

  Serial.printf("\033[33mSSID                  : \033[32m%s\033[0m\n", ssid);
  Serial.printf("\033[33mPassword              : \033[32m%s\033[0m\n", password);
  Serial.printf("\033[33mAI Model              : \033[32m%s\033[0m\n", model);
  Serial.printf("\033[33mAPI Key               : \033[32m%s\033[0m\n", token);
  Serial.printf("\033[33mSystem Inst.          : \033[32m%s\033[0m\n", systemInstruction);
  Serial.printf("\033[33mMax Tokens            : \033[32m%d\033[0m\n", maxTokens);
  Serial.printf("\033[33mTemperature           : \033[32m%.2f\033[0m\n", temperature);
  Serial.printf("\033[33mTopP                  : \033[32m%.2f\033[0m\n", TopP);
  Serial.printf("\033[33mTopK                  : \033[32m%.2f\033[0m\n", TopK);
  Serial.printf("\033[33mCode Execution        : \033[%sm%s\033[0m\n", codeExecution ? "32" : "31", codeExecution ? "Enabled" : "Disabled");
  Serial.printf("\033[33mGoogle Search         : \033[%sm%s\033[0m\n", googleSearch ? "32" : "31", googleSearch ? "Enabled" : "Disabled");
  Serial.printf("\033[33mLED Indicator         : \033[%sm%s\033[0m\n", ledmode ? "32" : "31", ledmode ? "Enabled" : "Disabled");

  Serial.println(F("\033[36m\n====================================================\033[0m\n"));
}

void saveToEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(0, EEPROM_MAGIC);
  int addr = 1;

  auto writeStr = [&](const char* s) {
    while (*s && addr < EEPROM_SIZE - 1) EEPROM.write(addr++, *s++);
    EEPROM.write(addr++, '\0');
  };

  writeStr(ssid);
  writeStr(password);
  writeStr(model);
  writeStr(token);
  writeStr(systemInstruction);

  EEPROM.put(addr, maxTokens); addr += sizeof(maxTokens);
  EEPROM.put(addr, temperature); addr += sizeof(temperature);
  EEPROM.put(addr, TopP); addr += sizeof(TopP);
  EEPROM.put(addr, TopK); addr += sizeof(TopK);
  EEPROM.put(addr, codeExecution); addr += sizeof(codeExecution);
  EEPROM.put(addr, googleSearch); addr += sizeof(googleSearch);
  EEPROM.put(addr, ledmode); addr += sizeof(ledmode);

  EEPROM.commit();
  EEPROM.end();
  Serial.println(F("Configuration saved to EEPROM."));
}

void loadFromEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(0) != EEPROM_MAGIC) {
    Serial.println(F("No EEPROM data."));
    EEPROM.end();
    return;
  }
  int addr = 1;
  auto readStr = [&]() {
    String s;
    while (addr < EEPROM_SIZE) {
      char c = EEPROM.read(addr++);
      if (c == '\0') break;
      s += c;
    }
    return s;
  };

  update("ssid", readStr());
  update("password", readStr());
  update("model", readStr());
  update("token", readStr());
  update("systemInstruction", readStr());

  EEPROM.get(addr, maxTokens); addr += sizeof(maxTokens);
  EEPROM.get(addr, temperature); addr += sizeof(temperature);
  EEPROM.get(addr, TopP); addr += sizeof(TopP);
  EEPROM.get(addr, TopK); addr += sizeof(TopK);
  EEPROM.get(addr, codeExecution); addr += sizeof(codeExecution);
  EEPROM.get(addr, googleSearch); addr += sizeof(googleSearch);
  EEPROM.get(addr, ledmode); addr += sizeof(ledmode);
  EEPROM.end();
}

void Commands(const String& input) {
  if (input == "rres") Serial.println(lastResponse);
  else if (input == "rreq") Serial.println(lastRequest);
  else if (input == "si" || input == "sysinfo") printSystemInfo();
  else if (input == "sc") showCredentials();
  else if (input.startsWith("cc ")) {
    String q = input.substring(3);
    q.trim();
    int first = q.indexOf('"');
    int second = q.indexOf('"', first + 1);
    int third = q.indexOf('"', second + 1);
    int fourth = q.indexOf('"', third + 1);
    if (first != -1 && second != -1 && third != -1 && fourth != -1) {
      String field = q.substring(first + 1, second);
      String value = q.substring(third + 1, fourth);
      update(field, value);
    } else Serial.println(F("Usage: cc \"field\" \"value\""));
  }
  else if (input == "s" || input == "save") saveToEEPROM();
  else if (input == "l" || input == "load") loadFromEEPROM();
  else if (input == "clear" || input == "cls") clear();
  else if (input == "h" || input == "help") printHelp();
}

void initEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(0) != EEPROM_MAGIC) {
    Serial.println(F("EEPROM uninitialized. Writing defaults..."));
    saveToEEPROM();
  } else {
    Serial.println(F("EEPROM already initialized."));
  }
  EEPROM.end();
}

void update(const String& key, const String& value) {
  bool valid = true;
  if (key == "ssid") {
    if (ssid) free(ssid);
    ssid = strdup(value.c_str());
  }
  else if (key == "password") {
    if (password) free(password);
    password = strdup(value.c_str());
  }
  else if (key == "model") {
    if (model) free(model);
    model = strdup(value.c_str());
  }
  else if (key == "token") {
    if (token) free(token);
    token = strdup(value.c_str());
  }
  else if (key == "systemInstruction") {
    if (systemInstruction) free(systemInstruction);
    systemInstruction = strdup(value.c_str());
  }
  else if (key == "maxTokens") maxTokens = value.toInt();
  else if (key == "temperature") temperature = value.toFloat();
  else if (key == "TopP") TopP = value.toFloat();
  else if (key == "TopK") TopK = value.toFloat();
  else if (key == "codeExecution") codeExecution = (value == "enable");
  else if (key == "googleSearch") googleSearch = (value == "enable");
  else if (key == "ledmode") ledmode = (value == "enable");
  else valid = false;

  if (valid) {
    Serial.printf("%s updated.\n", key.c_str());
    saveToEEPROM();
  } else {
    Serial.println(F("Invalid field."));
  }
}

void printHelp() {
  Serial.println(F("\033[36m\n================[ Available Commands ]================\033[0m\n"));
  Serial.println(F("rres               : print last JSON response"));
  Serial.println(F("rreq               : print last JSON request"));
  Serial.println(F("sysinfo,si         : system info"));
  Serial.println(F("sc                 : show current credentials"));
  Serial.println(F("cc                 : change credential: cc \"field\" \"value\""));
  Serial.println(F("save,s             : save to EEPROM"));
  Serial.println(F("load,l             : load from EEPROM"));
  Serial.println(F("clear,cls          : clear the screen"));
  Serial.println(F("help,h             : this help"));
  Serial.println(F("\033[36m\n====================================================\033[0m\n"));
}

void clear() {
  for (int i = 0; i < 100; i++) Serial.print('#');
  for (int i = 0; i < 50; i++) Serial.println();
}

String getAnswer(const String& question) {
  if (question == "rres" || question == "rreq" || question == "si" || question == "sc" || question.startsWith("cc") || question == "s" || question == "l" || question == "clear" || question == "h") {
    Commands(question);
    return String();
  }
  return gemini->getAnswer(question);
}

void loop() {
  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
      Serial.print(F(">> "));
      return;
    }
    String geminiSerial = getAnswer(line);
    if (geminiSerial.length()) {
      size_t totalLen = geminiSerial.length();
      for (size_t i = 0; i < totalLen; i += 64) {
        Serial.print(geminiSerial.substring(i, i + 64));
        delay(100);
        yield();
      }
      Serial.println();
    }
    Serial.print(F(">> "));
  }
}
