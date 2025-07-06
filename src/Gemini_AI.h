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

#pragma once

#include <Arduino.h>
#include <WiFiClientSecure.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <ESP8266SAM.h>
  #include "AudioOutputI2SNoDAC.h"
#else
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

class Gemini_AI {
  public:
    const char*  ssid;
    const char*  password;
    const char*  model;
    const char*  token;
    const char*  systemInstruction;
    int    maxTokens;
    float  temperature;
    float  TopP;
    float  TopK;
    bool   codeExecution;
    bool   googleSearch;
    bool   ledmode;
    Gemini_AI();
    ~Gemini_AI();
    bool connectToWiFi();
    String getAnswer(const String& question);
    String sayAnswer(const String& question);
    void say(const String& text);

  private:
#ifdef ESP8266
    AudioOutputI2SNoDAC *out = NULL;
    ESP8266SAM *sam = NULL;
#endif
    String response;
    String _sendRequest(const String& question);
    String _extractContent(const String& response);
    String _decodeUnicode(const String& input);
    void _sayInChunks(const String& text);
    String _escape(const String& s, bool encode);
};