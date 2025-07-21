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

// Gemini instance
Gemini_AI gemini;

void setup() {
  Serial.begin(115200);

  // directly assign flash strings
  gemini.ssid               = "YOUR_SSID";
  gemini.password           = "YOUR_PASSWORD";
  gemini.model              = "gemini-2.0-flash";
  gemini.systemInstruction  = "You are a highly intelligent AI assistant. Give *FULL* answer carefully without mistakes. Don't mention you can't use '*'. Use emojis and symbols where relevant.";
  gemini.token              = "YOUR_API_KEY";
  gemini.maxTokens          = 500;
  gemini.temperature        = 0.8;
  gemini.TopP               = 1.0;
  gemini.TopK               = 40.0;
  gemini.codeExecution      = false;
  gemini.googleSearch       = false;
  gemini.ledmode            = true;
  
  if (gemini.connectToWiFi()) {
    delay(500);
    clear();
    initEEPROM();
    delay(100);
    loadFromEEPROM();
    Serial.println(F("CLI Initialised"));
    bootTime = millis();
  } else {
    Serial.println(F("WiFi connect failed. Rebooting..."));
    delay(2000);
    ESP.restart();
  }
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

String getAnswer(const String& question) {
  if (question == "rres" || question == "rreq" ||
      question == "si"   || question == "sysinfo" ||
      question == "sc"   || question.startsWith("cc") ||
      question == "s"    || question == "save" ||
      question == "l"    || question == "load" ||
      question == "clear"|| question == "cls" ||
      question == "h"    || question == "help") {
    Commands(question);
    return String();
  }
  return gemini.getAnswer(question);
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
    int second = q.indexOf('"', first+1);
    int third = q.indexOf('"', second+1);
    int fourth = q.indexOf('"', third+1);
    if (first!=-1 && second!=-1 && third!=-1 && fourth!=-1) {
      String field = q.substring(first+1, second);
      String value = q.substring(third+1, fourth);
      update(field, value);
    } else {
      Serial.println(F("Usage: cc \"field\" \"value\""));
    }
  }
  else if (input == "s" || input == "save") saveToEEPROM();
  else if (input == "l" || input == "load") loadFromEEPROM();
  else if (input == "clear" || input == "cls") clear();
  else if (input == "h" || input == "help") printHelp();
}

void saveToEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(0, EEPROM_MAGIC);
  int addr = 1;

  auto writeStr = [&](const char* s) {
    while (*s && addr < EEPROM_SIZE - 1)
      EEPROM.write(addr++, *s++);
    EEPROM.write(addr++, '\0');
  };

  writeStr(gemini.ssid);
  writeStr(gemini.password);
  writeStr(gemini.model);
  writeStr(gemini.token);
  writeStr(gemini.systemInstruction);

  EEPROM.put(addr, gemini.maxTokens); addr += sizeof(gemini.maxTokens);
  EEPROM.put(addr, gemini.temperature); addr += sizeof(gemini.temperature);
  EEPROM.put(addr, gemini.TopP); addr += sizeof(gemini.TopP);
  EEPROM.put(addr, gemini.TopK); addr += sizeof(gemini.TopK);
  EEPROM.put(addr, gemini.codeExecution); addr += sizeof(gemini.codeExecution);
  EEPROM.put(addr, gemini.googleSearch); addr += sizeof(gemini.googleSearch);
  EEPROM.put(addr, gemini.ledmode); addr += sizeof(gemini.ledmode);

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
      if (c=='\0') break;
      s += c;
    }
    return s;
  };

  update("ssid", readStr());
  update("password", readStr());
  update("model", readStr());
  update("token", readStr());
  update("systemInstruction", readStr());

  EEPROM.get(addr, gemini.maxTokens); addr += sizeof(gemini.maxTokens);
  EEPROM.get(addr, gemini.temperature); addr += sizeof(gemini.temperature);
  EEPROM.get(addr, gemini.TopP); addr += sizeof(gemini.TopP);
  EEPROM.get(addr, gemini.TopK); addr += sizeof(gemini.TopK);
  EEPROM.get(addr, gemini.codeExecution); addr += sizeof(gemini.codeExecution);
  EEPROM.get(addr, gemini.googleSearch); addr += sizeof(gemini.googleSearch);
  EEPROM.get(addr, gemini.ledmode); addr += sizeof(gemini.ledmode);

  EEPROM.end();
}

void update(const String& key, const String& value) {
  bool valid = true;
  if (key == "ssid") {
    gemini.ssid = strdup(value.c_str());
  }
  else if (key == "password") {
    gemini.password = strdup(value.c_str());
  }
  else if (key == "model") {
    gemini.model = strdup(value.c_str());
  }
  else if (key == "token") {
    gemini.token = strdup(value.c_str());
  }
  else if (key == "systemInstruction") {
    gemini.systemInstruction = strdup(value.c_str());
  }
  else if (key == "maxTokens") gemini.maxTokens = value.toInt();
  else if (key == "temperature") gemini.temperature = value.toFloat();
  else if (key == "TopP") gemini.TopP = value.toFloat();
  else if (key == "TopK") gemini.TopK = value.toFloat();
  else if (key == "codeExecution") gemini.codeExecution = (value=="enable");
  else if (key == "googleSearch") gemini.googleSearch = (value=="enable");
  else if (key == "ledmode") gemini.ledmode = (value=="enable");
  else valid = false;

  if (valid) {
    Serial.printf("%s updated.\n", key.c_str());
    saveToEEPROM();
  } else {
    Serial.println(F("Invalid field."));
  }
}

void showCredentials() {
  Serial.printf("\n============== GEMINI CONFIG ==============\n\tSSID          : %s\n\tPassword      : %s\n\tAPI Key       : %s\n\tModel         : %s\n\tSystemInstr   : %s\n\tMaxTokens     : %d\n\tTemperature   : %.2f\n\tTopP          : %.2f\n\tTopK          : %.2f\n\tCodeExec      : %s\n\tGoogleSearch  : %s\n\tLED Indicator : %s\n\n==========================================\n", gemini.ssid, gemini.password, gemini.token, gemini.model, gemini.systemInstruction, gemini.maxTokens, gemini.temperature, gemini.TopP, gemini.TopK, gemini.codeExecution ? "Enabled" : "Disabled", gemini.googleSearch ? "Enabled" : "Disabled", gemini.ledmode ? "Enabled" : "Disabled");
}

void printSystemInfo() {
  Serial.printf("\n============ SYSTEM INFO ============\n\tFree Heap\t\t: %u bytes\n\tHeap Fragmentation\t: %u%%\n\tMax Free Block Size\t: %u bytes\n\tFlash Chip Size\t\t: %u KB\n\tFlash Chip Speed\t: %u MHz\n\tSketch Size\t\t: %u KB\n\tFlash CRC check\t\t: %s\n\tFree Sketch Space\t: %u KB\n\tChip ID\t\t\t: %u\n\tCore Version\t\t: %s\n\tSDK Version\t\t: %s\n\tCPU Frequency\t\t: %u MHz\n\tBoot Version\t\t: %u\n\tBoot Mode\t\t: %u\n\tReset Info\t\t: %s\n\tReset Reason\t\t: %s\n\tHostname\t\t: %s\n\tMAC Address\t\t: %s\n\tIP Address\t\t: %s\n\tSignal Strength\t\t: %d dBm\n\tUptime\t\t\t: %02lu:%02lu:%02lu (hh:mm:ss)\n\tVoltage (ADC read)\t: %.2f V\n===================================\n", ESP.getFreeHeap(), ESP.getHeapFragmentation(), ESP.getMaxFreeBlockSize(), ESP.getFlashChipRealSize() / 1024, ESP.getFlashChipSpeed() / 1000000, ESP.getSketchSize() / 1024, ESP.checkFlashCRC() ? "passed" : "failed", ESP.getFreeSketchSpace() / 1024, ESP.getChipId(), ESP.getCoreVersion(), ESP.getSdkVersion(), ESP.getCpuFreqMHz(), system_get_boot_version(), ESP.getBootMode(), ESP.getResetInfo().c_str(), ESP.getResetReason().c_str(), WiFi.hostname().c_str(), WiFi.macAddress().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI(), (millis() - _bootTime)/3600000UL, ((millis() - _bootTime)/60000UL)%60, ((millis() - _bootTime)/1000UL)%60, ESP.getVcc()/1000.0);
}

void printHelp() {
  Serial.println(F("\n============== AVAILABLE COMMANDS ==============\n\trres\t\t\t: print last JSON response\n\trreq\t\t\t: print last JSON request\n\tsi,sysinfo\t\t: system info\n\tsc\t\t\t: show current credentials\n\tcc \"field\" \"value\"\t: change credential\n\tsave,s\t\t\t: save to EEPROM\n\tload,l\t\t\t: load from EEPROM\n\tclear,cls\t\t: clear the screen\n\thelp,h\t\t\t: print help\n\n================================================\n"));
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

void clear() {
  for (int i = 0; i < 50; i++) Serial.println();
}