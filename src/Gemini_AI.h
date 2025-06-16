MIT License

Copyright (c) 2025 zacode123

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <WiFiClientSecure.h>
#include <cstring>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#else
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

//#if GEMINI_SERIAL == GET
  #define geminiSerialMode 1
//#elif GEMINI_SERIAL == SAY
  //#define geminiSerialMode 2
//#else
  //#error "Invalid GEMINI_SERIAL option. Use 'SAY' or 'GET'."
//#endif

//#ifdef USE_CLI
  //#define USE_CLI
//#endif

#ifdef USE_CLI

#ifndef EEPROM_SIZE
#define EEPROM_SIZE 1024
#endif
#ifndef EEPROM_MAGIC
#define EEPROM_MAGIC 0x42
#endif
#endif
class Gemini_AI {
  public:
    Gemini_AI(const char* ssid, const char* password, const char* model, const char* token, int maxTokens, const char* systemInstruction, float temperature, float TopP, float TopK, bool codeExecution, bool googleSearch, bool ledmode);
    ~Gemini_AI();
    bool connectToWiFi();
    String getAnswer(const String& question);
    String sayAnswer(const String& question);
    void loop();
#ifdef USE_CLI
    void saveToEEPROM();
    void loadFromEEPROM();
#endif

#ifdef geminiSerialMode
    String geminiSerial;
#endif

  private:
    char*  _ssid;
    char*  _password;
    char*  _model;
    char*  _token;
    char*  _systemInstruction;
    int    _maxTokens;
    float  _temperature;
    float  _TopP;
    float  _TopK;
    bool   _codeExecution;
    bool   _googleSearch;
    bool   _ledmode;
    String _lastResponse;
#ifdef USE_CLI
    unsigned long _bootTime;
    String _lastRequest;
    void _printHelp();
    void _printSystemInfo();
    void _showCredentials();
    void _update(const String& key, const String& value);
    void _Commands(const String& command);
    void _initEEPROM();

#endif
    static char* _strdup_safe(const char* s);
    bool _sendRequest(const String& question, String& response);
    String _getResponse(const String& question);
    String _extractContent(const String& response);
    String _decodeUnicode(const String& input);
    void _sayInChunks(const String& text, size_t chunkSize = 256);
    void _clear();
};
