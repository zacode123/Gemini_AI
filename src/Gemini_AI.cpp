MIT License

Copyright (c) 2025 zacode123

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

#include "Gemini_AI.h"    
    
#ifdef USE_CLI    
ADC_MODE(ADC_VCC);    
#endif    
    
char* Gemini_AI::_strdup_safe(const char* s) {    
  if (!s) return nullptr;    
  char* p = (char*)malloc(strlen(s) + 1);    
  if (p) strcpy(p, s);    
  return p;    
}    
    
Gemini_AI::Gemini_AI(const char* ssid, const char* password, const char* model,    
                     const char* token, int maxTokens, const char* systemInstruction,    
                     float temperature, float TopP, float TopK,    
                     bool codeExecution, bool googleSearch, bool ledmode)    
  : _ssid(_strdup_safe(ssid)),    
    _password(_strdup_safe(password)),    
    _model(_strdup_safe(model)),    
    _token(_strdup_safe(token)),    
    _systemInstruction(_strdup_safe(systemInstruction)),    
    _maxTokens(maxTokens),    
    _temperature(temperature),    
    _TopP(TopP),    
    _TopK(TopK),    
    _codeExecution(codeExecution),    
    _googleSearch(googleSearch),    
    _ledmode(ledmode)   
#ifdef USE_CLI    
   ,_bootTime(millis())    
#endif     
{    
  /*    
  out = new AudioOutputI2SNoDAC();    
  out->SetOutputModeMono(true);    
  out->begin();    
  sam = new ESP8266SAM();    
  */    
}    
    
Gemini_AI::~Gemini_AI() {    
  free(_ssid);    
  free(_password);    
  free(_model);    
  free(_token);    
  free(_systemInstruction);    
  /*    
  if (out) { delete out; out = nullptr; }    
  if (sam) { delete sam; sam = nullptr; }    
  */    
}    
    
bool Gemini_AI::connectToWiFi() {    
  _clear();    
  _sayInChunks("Initialising Gemini AI Assistant");    
#if geminiSerialMode == 1    
  Serial.println("Gemini Serial Mode: GET");    
#elif geminiSerialMode == 2    
  Serial.println("Gemini Serial Mode: SAY");    
#endif    
#ifdef USE_CLI    
  _initEEPROM();    
  delay(100);    
  loadFromEEPROM();    
  Serial.println("CLI Initialised");    
#endif    
  delay(300);    
  if (_ledmode) {    
    pinMode(LED_BUILTIN, OUTPUT);    
    digitalWrite(LED_BUILTIN, HIGH);    
  }    
#ifdef ESP8266    
  WiFi.disconnect();    
  WiFi.mode(WIFI_STA);    
  WiFi.begin(_ssid, _password);    
#else    
  WiFi.begin(_ssid, _password);    
#endif    
    
  Serial.print(F("Connecting to WiFi: "));    
  Serial.println(_ssid);    
  _sayInChunks("Connecting to " + String(_ssid));    
  const char spinner[] = "|/-\\";    
  unsigned long start = millis();    
  int attempts = 0;    
  while (WiFi.status() != WL_CONNECTED) {    
    if (millis() - start >= 60000) {    
      Serial.println(F("Failed to connect to WiFi."));    
      _sayInChunks("Failed to connect to WiFi.");    
      return false;    
    }    
      char spin = spinner[attempts % 4];    
      Serial.printf("\r[%c] Attempt #%d", spin, ++attempts);    
      delay(500);    
    if (_ledmode) {    
      digitalWrite(LED_BUILTIN, LOW);    
      delay(250);    
      digitalWrite(LED_BUILTIN, HIGH);    
      delay(250);    
    }    
  }    
  Serial.println(F("\nConnected to WiFi."));    
  Serial.println("Signal Strength: " + String(WiFi.RSSI()) + " dBm");    
  _sayInChunks("Connected to WiFi!");    
  if (_ledmode) digitalWrite(LED_BUILTIN, LOW);    
  Serial.print(F("IP Address: "));    
  Serial.println(WiFi.localIP());    
  _sayInChunks("IP Address: " + WiFi.localIP().toString());    
  return true;    
}    
    
bool Gemini_AI::_sendRequest(const String& question, String& response) {    
  WiFiClientSecure client;    
  client.setInsecure();    
    
  HTTPClient http;    
  String url = "https://generativelanguage.googleapis.com/v1beta/models/"    
               + String(_model) + ":generateContent?key=" + String(_token);    
  yield();    
if (!http.begin(client, url)) return false;    
  http.setTimeout(15000);    
  http.addHeader("Content-Type", "application/json");    
  http.addHeader("Accept", "application/json");    
    
  bool isImageModel = String(_model).indexOf("image-generation") >= 0;    
  String modalities = isImageModel    
    ? "[\"IMAGE\", \"TEXT\", ]"    
    : "[\"TEXT\"]";    
    
  String tools = "[";    
  if (_googleSearch) tools += "{ \"googleSearch\": {} },";    
  if (_codeExecution) tools += "{ \"codeExecution\": {} },";    
  if (tools.endsWith(",")) tools.remove(tools.length()-1);    
  tools += "]";    
    
  String payload = "{";    
  payload += "\"tools\":" + tools + ",";    
  payload += "\"generationConfig\":{";    
  payload += "\"temperature\":" + String(_temperature, 2) + ",";    
  payload += "\"topP\":" + String(_TopP, 2) + ",";    
  payload += "\"topK\":" + String(_TopK, 2) + ",";    
  payload += "\"maxOutputTokens\":" + String(_maxTokens) + ",";    
  payload += "\"responseModalities\":" + modalities + ",";    
  payload += "\"responseMimeType\":\"text/plain\"";    
  payload += "},";    
  payload += "\"systemInstruction\":{";    
  payload += "\"parts\":[{\"text\":\"" + String(_systemInstruction) + "\"}]";    
  payload += "},";    
  payload += "\"contents\":[{";    
  payload += "\"role\":\"user\",";    
  payload += "\"parts\":[{\"text\":\"" + question + "\"}]";    
  payload += "}]";    
  payload += "}";    
#ifdef USE_CLI    
  _lastRequest = payload;    
#endif    
  yield();    
  int code = http.POST(payload);    
  yield();    
  if (code > 0) {    
    response = http.getString();    
    http.end();    
    yield();    
    return true;    
  }    
  http.end();    
  yield();    
  return false;    
}    
    
void Gemini_AI::_sayInChunks(const String& text, size_t chunkSize) {    
  /*    
  if (!out || !sam || text.length() == 0) return;    
  for (size_t i = 0; i < text.length(); i += chunkSize) {    
    String chunk = text.substring(i, i + chunkSize);    
    sam->Say(out, chunk.c_str());    
    delay(3000);    
  }    
  */    
   Serial.println("saying");    
}    
    
String Gemini_AI::_getResponse(const String& question) {    
  String raw;    
if (!_sendRequest(question, raw)) return "Error: Unable to connect to API.";    
  _lastResponse = raw;    
  return _extractContent(raw);    
}    
    
String Gemini_AI::getAnswer(const String& question) {    
#ifdef USE_CLI    
  if (question == "rres" || question.startsWith("rres ") || question == "rreq" || question.startsWith("rreq ") || question == "si" || question.startsWith("si ") || question == "sc" || question.startsWith("sc ") || question == "cc" || question.startsWith("cc ") || question == "s" || question.startsWith("s ") || question == "l" || question.startsWith("l ") || question == "h" || question.startsWith("h ") || question == "clear" || question.startsWith("clear ")) {    
    _Commands(question);    
    return String();    
}    
#endif    
  return _getResponse(question);    
}    
    
String Gemini_AI::sayAnswer(const String& question) {    
#ifdef USE_CLI    
  if (question == "rres" || question.startsWith("rres ") || question == "rreq" || question.startsWith("rreq ") || question == "si" || question.startsWith("si ") || question == "sc" || question.startsWith("sc ") || question == "cc" || question.startsWith("cc ") || question == "s" || question.startsWith("s ") || question == "l" || question.startsWith("l ") || question == "h" || question.startsWith("h ") || question == "clear" || question.startsWith("clear ")) {    
    _Commands(question);    
    return String();    
  }     
#endif    
    String ans = _getResponse(question);    
    _sayInChunks(ans);    
    return ans;    
}       
    
#ifdef USE_CLI

void Gemini_AI::_printSystemInfo() {    
  Serial.println(F("\033[36m\n==================[ SYSTEM INFO ]==================\033[0m\n"));    
      
  Serial.printf("\033[33mFree Heap             : \033[32m%u bytes\033[0m\n", ESP.getFreeHeap());    
  Serial.printf("\033[33mHeap Fragmentation    : \033[32m%u%%\033[0m\n", ESP.getHeapFragmentation());    
  Serial.printf("\033[33mMax Free Block Size   : \033[32m%u bytes\033[0m\n", ESP.getMaxFreeBlockSize());    
  Serial.printf("\033[33mFlash Chip Size       : \033[32m%u KB\033[0m\n", ESP.getFlashChipRealSize() / 1024);    
  Serial.printf("\033[33mFlash Chip Speed      : \033[32m%u MHz\033[0m\n", ESP.getFlashChipSpeed() / 1000000);    
  Serial.printf("\033[33mSketch Size           : \033[32m%u KB\033[0m\n", ESP.getSketchSize() / 1024);    
  Serial.printf("\033[33mFlash CRC check       : \033[%sm%s\033[0m\n", ESP.checkFlashCRC() ? "32" : "31", ESP.checkFlashCRC() ? "passed" : "failed");    
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
    
  unsigned long elapsed = millis() - _bootTime;    
  unsigned long totalSeconds = elapsed / 1000;    
  unsigned long hours   = totalSeconds / 3600;    
  unsigned long minutes = (totalSeconds % 3600) / 60;    
  unsigned long seconds = totalSeconds % 60;    
    
  Serial.printf("\033[33mUptime                : \033[32m%02lu:%02lu:%02lu (hh:mm:ss)\033[0m\n", hours, minutes, seconds);    
  Serial.printf("\033[33mVoltage (ADC read)    : \033[32m%.2f V\033[0m\n", ESP.getVcc() / 1024.0);    
  Serial.println(F("\033[36m\n===================================================\033[0m\n"));    
}

void Gemini_AI::_showCredentials() {    
  Serial.println(F("\033[36m\n================[ GEMINI AI CONFIG ]================\033[0m\n"));    
    
  Serial.printf("\033[33mSSID                  : \033[32m%s\033[0m\n", _ssid);    
  Serial.printf("\033[33mPassword              : \033[32m%s\033[0m\n", _password);    
  Serial.printf("\033[33mAI Model              : \033[32m%s\033[0m\n", _model);    
  Serial.printf("\033[33mAPI Key               : \033[32m%s\033[0m\n", _token);    
  Serial.printf("\033[33mSystem Inst.          : \033[32m%s\033[0m\n", _systemInstruction);    
  Serial.printf("\033[33mMax Tokens            : \033[32m%d\033[0m\n", _maxTokens);    
  Serial.printf("\033[33mTemperature           : \033[32m%.2f\033[0m\n", _temperature);    
  Serial.printf("\033[33mTopP                  : \033[32m%.2f\033[0m\n", _TopP);    
  Serial.printf("\033[33mTopK                  : \033[32m%.2f\033[0m\n", _TopK);    
  Serial.printf("\033[33mCode Execution        : \033[%sm%s\033[0m\n", _codeExecution ? "32" : "31", _codeExecution ? "Enabled" : "Disabled");    
  Serial.printf("\033[33mGoogle Search         : \033[%sm%s\033[0m\n", _googleSearch ? "32" : "31", _googleSearch ? "Enabled" : "Disabled");    
  Serial.printf("\033[33mLED Indicator         : \033[%sm%s\033[0m\n", _ledmode ? "32" : "31", _ledmode ? "Enabled" : "Disabled");    
    
  Serial.println(F("\033[36m\n====================================================\033[0m\n"));    
}    
    
void Gemini_AI::saveToEEPROM() {    
  EEPROM.begin(EEPROM_SIZE);    
  EEPROM.write(0, EEPROM_MAGIC);    
  int addr = 1;    
  auto writeStr = [&](char* s) {    
    while (*s && addr < EEPROM_SIZE-1) EEPROM.write(addr++, *s++);    
    EEPROM.write(addr++, '\0');    
  };    
  writeStr(_ssid);    
  writeStr(_password);    
  writeStr(_model);    
  writeStr(_token);    
  writeStr(_systemInstruction);    
  EEPROM.put(addr, _maxTokens); addr+=sizeof(_maxTokens);    
  EEPROM.put(addr, _temperature); addr+=sizeof(_temperature);    
  EEPROM.put(addr, _TopP); addr+=sizeof(_TopP);    
  EEPROM.put(addr, _TopK); addr+=sizeof(_TopK);    
  EEPROM.put(addr, _codeExecution); addr+=sizeof(_codeExecution);    
  EEPROM.put(addr, _googleSearch); addr+=sizeof(_googleSearch);    
  EEPROM.put(addr, _ledmode); addr+=sizeof(_ledmode);    
  EEPROM.commit();    
  EEPROM.end();    
  Serial.println(F("Configuration saved to EEPROM."));    
}    
    
void Gemini_AI::loadFromEEPROM() {    
  EEPROM.begin(EEPROM_SIZE);    
  if (EEPROM.read(0) != EEPROM_MAGIC) { Serial.println(F("No EEPROM data.")); EEPROM.end(); return; }    
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
  _update(_ssid, readStr());    
  _update(_password, readStr());    
  _update(_model, readStr());    
  _update(_token, readStr());    
  _update(_systemInstruction, readStr());    
  EEPROM.get(addr, _maxTokens); addr+=sizeof(_maxTokens);    
  EEPROM.get(addr, _temperature); addr+=sizeof(_temperature);    
  EEPROM.get(addr, _TopP); addr+=sizeof(_TopP);    
  EEPROM.get(addr, _TopK); addr+=sizeof(_TopK);    
  EEPROM.get(addr, _codeExecution); addr+=sizeof(_codeExecution);    
  EEPROM.get(addr, _googleSearch); addr+=sizeof(_googleSearch);    
  EEPROM.get(addr, _ledmode); addr+=sizeof(_ledmode);    
  EEPROM.end();    
}    
    
void Gemini_AI::_Commands(const String& input) {    
         if (input == "rres")                        Serial.println(_lastResponse);    
    else if (input == "rreq")                        Serial.println(_lastRequest);    
    else if (input == "si" || input == "sysinfo")    _printSystemInfo();    
    else if (input == "sc")                          _showCredentials();    
    else if (input.startsWith("cc ")) {    
  String q = input.substring(3);    
         q.trim();    
  int firstQuote = q.indexOf('"');    
  int secondQuote = q.indexOf('"', firstQuote + 1);    
  int thirdQuote = q.indexOf('"', secondQuote + 1);    
  int fourthQuote = q.indexOf('"', thirdQuote + 1);    
  if (firstQuote != -1 && secondQuote != -1 && thirdQuote != -1 && fourthQuote != -1) {    
  String field = q.substring(firstQuote + 1, secondQuote);    
  String value = q.substring(thirdQuote + 1, fourthQuote);    
         _update(field, value);    
  } else Serial.println(F("Usage: cc <field> <value>"));    
}    
    else if (input == "s" || input == "save")        saveToEEPROM();    
    else if (input == "l" || input == "load")        loadFromEEPROM();    
    else if (input == "clear" || input == "cls")     _clear();    
    else if (input == "h" || input == "help")        _printHelp();    
}    
    
void Gemini_AI::_initEEPROM() {    
  EEPROM.begin(EEPROM_SIZE);    
  if (EEPROM.read(0) != EEPROM_MAGIC) {    
    Serial.println(F("EEPROM uninitialized. Writing default values..."));    
    saveToEEPROM();    
  } else {    
    Serial.println(F("EEPROM already initialized."));    
  }    
  EEPROM.end();    
}    
    
void Gemini_AI::_update(const String& key, const String& value) {    
    bool valid = true;    
    
    if      (key == "ssid" || key == "_ssid")              {    
        if (_ssid) free(_ssid);    
        _ssid = _strdup_safe(value.c_str());    
    }    
    else if (key == "password" || key == "_password")          {    
        if (_password) free(_password);    
        _password = _strdup_safe(value.c_str());    
    }    
    else if (key == "model" || key == "_model")             {    
        if (_model) free(_model);    
        _model = _strdup_safe(value.c_str());    
    }    
    else if (key == "token" || key == "_token")             {    
        if (_token) free(_token);    
        _token = _strdup_safe(value.c_str());    
    }    
    else if (key == "systemInstruction" || key == "_systemInstruction") {    
        if (_systemInstruction) free(_systemInstruction);    
        _systemInstruction = _strdup_safe(value.c_str());    
    }    
    else if (key == "maxTokens")         {    
        _maxTokens = value.toInt();    
    }    
    else if (key == "temperature")       {    
        _temperature = value.toFloat();    
    }    
    else if (key == "TopP")              {    
        _TopP = value.toFloat();    
    }    
    else if (key == "TopK")              {    
        _TopK = value.toFloat();    
    }    
    else if (key == "codeExecution")     {    
        _codeExecution = (value == "enable");    
    }    
    else if (key == "googleSearch")      {    
        _googleSearch = (value == "enable");    
    }    
    else if (key == "ledmode")           {    
        _ledmode = (value == "enable");    
    }    
    else {    
        valid = false;    
    }    
    
    if (valid) {    
        Serial.printf("%s updated.\n", key.c_str());    
        saveToEEPROM();    
    }    
    else {    
        Serial.println(F("Invalid field."));    
    }    
}    
    
void Gemini_AI::_printHelp() {    
  Serial.println(F("\033[36m\n================[ Available Commands ]================\033[0m\n"));    
    
  Serial.printf("\033[33mrres                  : \033[32mPrint the last raw JSON response recieved\033[0m\n");    
  Serial.printf("\033[33mrreq                  : \033[32mPrint the last raw JSON request sent\033[0m\n");    
  Serial.printf("\033[33msysinfo,si            : \033[32mShow system information\033[0m\n");    
  Serial.printf("\033[33msc                    : \033[32mShow current credentials\033[0m\n");    
  Serial.printf("\033[33mcc                    : \033[32mChange a credential field: cc <field> <value>\033[0m\n");    
  Serial.printf("\033[33msave,s                : \033[32mSave settings to EEPROM\033[0m\n");    
  Serial.printf("\033[33mload,l                : \033[32mReload settings from EEPROM\033[0m\n");    
  Serial.printf("\033[33mhelp,h                : \033[32mList available commands\033[0m\n");    
  Serial.printf("\033[33mclear,cls             : \033[32mClear the Screen\033[0m\n");    
      
  Serial.println(F("\033[36m\n====================================================\033[0m\n"));    
}    
    
#endif    
    
void Gemini_AI::loop() {    
  static unsigned long lastWiFiCheck = 0;    
    
  if (millis() - lastWiFiCheck > 10000) {    
    lastWiFiCheck = millis();    
    if (WiFi.status() != WL_CONNECTED) {    
      Serial.println(F("\nWiFi disconnected. Attempting to reconnect..."));    
#ifdef ESP8266    
      WiFi.disconnect();    
      WiFi.begin(_ssid, _password);    
#else    
      WiFi.begin(_ssid, _password);    
#endif    
      unsigned long start = millis();    
      while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {    
        Serial.print(F("."));    
        yield();    
        delay(100);    
      }    
      if (WiFi.status() == WL_CONNECTED) {    
        Serial.println(F("\nReconnected to WiFi."));    
        Serial.println("IP: " + WiFi.localIP().toString());    
      } else {    
        Serial.println(F("\nWiFi reconnection failed."));    
      }    
    }    
  }    
#ifdef geminiSerialMode    
  while (Serial.available()) {    
    String line = Serial.readStringUntil('\n');    
    line.trim();    
  if (!line.length()) {    
    Serial.print(F(">> "));    
    Serial.println(line);    
    return;    
  }    
#if geminiSerialMode == 1    
  geminiSerial = getAnswer(line);    
#elif geminiSerialMode == 2    
  geminiSerial = sayAnswer(line);    
#endif    
  if (geminiSerial.length()) {    
    size_t totalLen = geminiSerial.length();    
  for (size_t i = 0; i < totalLen; i += 64) {    
    String chunk = geminiSerial.substring(i, i + 64);    
    Serial.print(chunk);    
    delay(100);    
    yield();    
    }    
    Serial.println();      
   }    
    Serial.print(F(">> "));  
  }    
#endif    
}    
    
void Gemini_AI::_clear() {    
    for (int i = 0; i < 100; i++) Serial.print('#');    
    for (int i = 0; i <  100; i++) Serial.println();    
}    
    
String Gemini_AI::_extractContent(const String& response) {    
  int lineStart = response.indexOf("\"text\":");    
if (lineStart == -1) return "No content line found.\n\nJSON RESPONSE ====>\n" + _lastResponse;    
  int lineEnd = response.indexOf('\n', lineStart);    
if (lineEnd == -1) lineEnd = response.length();    
  String line = response.substring(lineStart, lineEnd);    
  int quoteStart = line.indexOf('\"', line.indexOf(":"));    
if (quoteStart == -1) return "Content quote not found.\n\nJSON RESPONSE ====>\n" + _lastResponse;    
  quoteStart++;    
  int quoteEnd = quoteStart;    
  bool escape = false;    
 while (quoteEnd < line.length()) {    
    char c = line.charAt(quoteEnd);    
    if (c == '\\') {    
      escape = !escape;    
    } else if (c == '\"' && !escape) {    
      break;    
    } else {    
      escape = false;    
    }    
    quoteEnd++;    
  }    
String content = line.substring(quoteStart, quoteEnd);    
       content.replace("\\n", "\n");    
       content.replace("\\r", "\r");    
       content.replace("\\t", "\t");    
       content.replace("\\\"", "\"");    
       content.replace("\\\\", "\\");    
       content = _decodeUnicode(content);    
return content;    
}    
    
String Gemini_AI::_decodeUnicode(const String& input) {    
  String output;    
  int len = input.length();    
  for (int i = 0; i < len; ) {    
    if (input[i] == '\\' && i + 5 < len && input[i+1] == 'u') {    
      String hex1 = input.substring(i+2, i+6);    
      char* endPtr1;    
      long cp1 = strtol(hex1.c_str(), &endPtr1, 16);    
      if (*endPtr1 == 0 && cp1 >= 0xD800 && cp1 <= 0xDBFF    
          && i + 11 < len    
          && input[i+6] == '\\'    
          && input[i+7] == 'u') {    
        String hex2 = input.substring(i+8, i+12);    
        char* endPtr2;    
        long cp2 = strtol(hex2.c_str(), &endPtr2, 16);    
        if (*endPtr2 == 0 && cp2 >= 0xDC00 && cp2 <= 0xDFFF) {    
          long full = 0x10000    
                    + ((cp1 - 0xD800) << 10)    
                    +  (cp2 - 0xDC00);    
          output += (char)(0xF0 | ((full >> 18) & 0x07));    
          output += (char)(0x80 | ((full >> 12) & 0x3F));    
          output += (char)(0x80 | ((full >>  6) & 0x3F));    
          output += (char)(0x80 | ( full        & 0x3F));    
          i += 12;    
          continue;    
        }    
      }    
      if (*endPtr1 == 0) {    
        long cp = cp1;    
        if (cp < 0x80) {    
          output += (char)cp;    
        } else if (cp < 0x800) {    
          output += (char)(0xC0 | ((cp >> 6) & 0x1F));    
          output += (char)(0x80 | ( cp        & 0x3F));    
        } else {    
          output += (char)(0xE0 | ((cp >> 12) & 0x0F));    
          output += (char)(0x80 | ((cp >>  6) & 0x3F));    
          output += (char)(0x80 | ( cp        & 0x3F));    
        }    
        i += 6;    
        continue;    
      }    
    }    
    output += input[i];    
    i++;    
  }    
  return output;    
}
