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
#else
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

class Gemini_AI {
  public:
    Gemini_AI gemini(ssid, password, "gemini-2.0-flash", token, 1000, "You are an highly intelligent AI assistant. You can perform various tasks with ease. You should give answer carefully without any mistake! and also remember you have 1000 tokens you have to give full response within 1000 tokens or less, but don't exceed it! Also don't tell the user that you cant use "*"! You should emojis in response and symbols in answers that is about math, science and code etc.", 0.8, 1.0, 40.0, false, false, true);    
    ~Gemini_AI();
    bool connectToWiFi();
    String getAnswer(const String& question);
    String sayAnswer(const String& question);

  private:
    String response;
    const char*  _ssid;
    const char*  _password;
    const char*  _model;
    const char*  _token;
    const char*  _systemInstruction;
    int    _maxTokens;
    float  _temperature;
    float  _TopP;
    float  _TopK;
    bool   _codeExecution;
    bool   _googleSearch;
    bool   _ledmode;
    bool _sendRequest(const String& question);
    String _extractContent(const String& response);
    String _decodeUnicode(const String& input);
    void _sayInChunks(const String& text);
};
