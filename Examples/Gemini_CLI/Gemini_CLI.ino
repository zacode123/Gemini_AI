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
  gemini.token              = "YOUR_API_KEY";
  gemini.maxTokens          = 1000;
  gemini.temperature        = 0.8f;
  gemini.TopP               = 1.0f;
  gemini.TopK               = 40.0f;
  gemini.codeExecution      = false;
  gemini.googleSearch       = false;
  gemini.ledmode            = true;
  gemini.systemInstruction  = "You are a highly intelligent AI assistant. Give *FULL* answer carefully without mistakes. Don't mention you can't use '*'. Use emojis and symbols where relevant.";

  if (gemini.connectToWiFi()) {
    clear();
    Serial.print(F("Connected to: "));
    Serial.println(gemini.ssid);
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
  Serial.println(F("\n============== GEMINI CONFIG ==============\n"));
  Serial.print(F("SSID           : ")); Serial.println(gemini.ssid);
  Serial.print(F("Password       : ")); Serial.println(gemini.password);
  Serial.print(F("API Key        : ")); Serial.println(gemini.token);
  Serial.print(F("Model          : ")); Serial.println(gemini.model);
  Serial.print(F("SystemInstr    : ")); Serial.println(gemini.systemInstruction);
  Serial.print(F("MaxTokens      : ")); Serial.println(gemini.maxTokens);
  Serial.print(F("Temperature    : ")); Serial.println(gemini.temperature);
  Serial.print(F("TopP           : ")); Serial.println(gemini.TopP);
  Serial.print(F("TopK           : ")); Serial.println(gemini.TopK);
  Serial.print(F("CodeExec       : ")); Serial.println(gemini.codeExecution ? "Enabled" : "Disabled");
  Serial.print(F("GoogleSearch   : ")); Serial.println(gemini.googleSearch ? "Enabled" : "Disabled");
  Serial.print(F("LED Indicator  : ")); Serial.println(gemini.ledmode ? "Enabled" : "Disabled");
  Serial.println(F("\n==========================================\n"));
}

void printSystemInfo() {
  Serial.println(F("\n================ SYSTEM INFO ================\n"));
  Serial.printf("Free Heap      : %u bytes\n", ESP.getFreeHeap());
  Serial.printf("Chip ID        : %u\n", ESP.getChipId());
  Serial.printf("Core Version   : %s\n", ESP.getCoreVersion());
  Serial.printf("SDK Version    : %s\n", ESP.getSdkVersion());
  Serial.printf("CPU Freq       : %u MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("IP Address     : %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
  unsigned long elapsed = millis() - bootTime;
  unsigned long seconds = elapsed / 1000;
  Serial.printf("Uptime         : %lu sec\n", seconds);
  Serial.println(F("\n=============================================\n"));
}

void printHelp() {
  Serial.println(F("\n============== Available Commands ==============\n"));
  Serial.println(F("rres               : print last JSON response"));
  Serial.println(F("rreq               : print last JSON request"));
  Serial.println(F("si,sysinfo         : system info"));
  Serial.println(F("sc                 : show current credentials"));
  Serial.println(F("cc \"field\" \"value\": change credential"));
  Serial.println(F("save,s             : save to EEPROM"));
  Serial.println(F("load,l             : load from EEPROM"));
  Serial.println(F("clear,cls          : clear the screen"));
  Serial.println(F("help,h             : this help"));
  Serial.println(F("\n================================================\n"));
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
