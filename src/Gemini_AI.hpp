/*  
 * Gemini_AI.hpp - A lightweight Gemini AI client library for ESP8266/ESP32 to get response from Google gemini AI over WiFi.  
 *   
 * @brief  Simple and memory-efficient Gemini AI Client with stream-based JSON parsing, WiFi handling, and optional TTS support for ESP8266.  
 * @author zacode123  
 * @date   16 July 2025  
 * @license MIT License  
 *  
 * MIT License  
 *   
 * Copyright (c) 2025 zacode123  
 *   
 * Permission is hereby granted, free of charge, to any person obtaining a copy  
 * of this software and associated documentation files (the "Software"), to deal  
 * in the Software without restriction, including without limitation the rights  
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell  
 * copies of the Software, and to permit persons to whom the Software is  
 * furnished to do so, subject to the following conditions:  
 */  
  
#pragma once  

#include <Arduino.h>  
#include <WiFiClientSecure.h>  
#include "ESP8266StreamJsonParser.hpp"
#include "ESP8266StaticJsonBuilder.hpp"

#ifdef ESP8266  
    #include <ESP8266WiFi.h>  
    #include <ESP8266HTTPClient.h>  
  #ifdef USE_TTS  
    #include <ESP8266SAM.h>  
    #include "AudioOutputI2SNoDAC.h"  
  #endif  
#else  
  #include <WiFi.h>  
  #include <HTTPClient.h>  
#endif  
  
class Gemini_AI {  
private:  
  #ifdef ESP8266  
   #ifdef USE_TTS  
     AudioOutputI2SNoDAC* out = nullptr;  
     ESP8266SAM* sam = nullptr;  
   #endif  
  #endif  
  
  String response;  
  int outlength;  
  
  String _escape(const String& s) {
    outlength = s.length() + (s.length() / 10);
    String out;
    if (!out.reserve(outlength)) {
      Serial.println("Error: Failed to reserve" + String(outlength) + "b for out in '_escape()'!");
    }
    for (char c : s) {
      switch (c) {
        case '\"': out.concat(F("\\\"")); break;
        case '\\': out.concat(F("\\\\")); break;
        case '\b': out.concat(F("\\b"));  break;
        case '\f': out.concat(F("\\f"));  break;
        case '\n': out.concat(F("\\n"));  break;
        case '\r': out.concat(F("\\r"));  break;
        case '\t': out.concat(F("\\t"));  break;
        default:   out += c; break;
      }
    }
    return out;
  }
  
  String _buildGeminiPayload(const String& question) {
    int maxtokens = std::min(maxTokens, MAX_TOKENS);
    char payload[PAYLOAD_BUFFER_SIZE];
    ESP8266StaticJsonBuilder builder(payload, PAYLOAD_BUFFER_SIZE, false);
    builder.beginObject();
    if (googleSearch || codeExecution) {
      builder.key("tools");
      builder.beginArray();
      if (googleSearch) {
        builder.beginObject();
        builder.key("googleSearch");
        builder.beginObject();
        builder.endObject();
        builder.endObject();
      }
      if (codeExecution) {
        builder.beginObject();
        builder.key("codeExecution");
        builder.beginObject();
        builder.endObject();
        builder.endObject();
      }
      builder.endArray();
    }
    builder.key("generationConfig");
    builder.beginObject();
    builder.key("temperature");
    builder.value(temperature);
    builder.key("topP");
    builder.value(TopP);
    builder.key("topK");
    builder.value(TopK);
    builder.key("maxOutputTokens");
    builder.value(maxtokens);
    if (strstr(model, "image-generation") != nullptr) {
      builder.key("responseModalities");
      builder.beginArray();
      builder.value("IMAGE");
      builder.value("TEXT");
      builder.endArray();
    }
    builder.key("responseMimeType");
    builder.value("text/plain");
    builder.endObject();
    builder.key("systemInstruction");
    builder.beginObject();
    builder.key("parts");
    builder.beginArray();
    builder.beginObject();
    builder.key("text");
    builder.value(_escape(String(systemInstruction)));
    builder.endObject();
    builder.endArray();
    builder.endObject();
    if (temperature != 0 || TopP != 0 || TopK != 0 || maxtokens != 0 || strstr(model, "image-generation") != nullptr) {
      builder.key("generationConfig");
      builder.beginObject();
      if (temperature != 0) {
        builder.key("temperature");
        builder.value(temperature);
      }
      if (TopP != 0) {
        builder.key("topP");
        builder.value(TopP);
      }
      if (TopK != 0) {
        builder.key("topK");
        builder.value(TopK);
      }
      if (maxtokens != 0) {
        builder.key("maxOutputTokens");
        builder.value(maxtokens);
      }
      if (strstr(model, "image-generation") != nullptr) {
        builder.key("responseModalities");
        builder.beginArray();
        builder.value("IMAGE");
        builder.value("TEXT");
        builder.endArray();
      }
      builder.endObject();
    }
    if (systemInstruction && strlen(systemInstruction) > 0) {
      builder.key("systemInstruction");
      builder.beginObject();
      builder.key("parts");
      builder.beginArray();
      builder.beginObject();
      builder.key("text");
      builder.value(_escape(String(systemInstruction)));
      builder.endObject();
      builder.endArray();
      builder.endObject();
    }
    builder.key("contents");
    builder.beginArray();
    builder.beginObject();
    builder.key("role");
    builder.value("user");
    builder.key("parts");
    builder.beginArray();
    builder.beginObject();
    builder.key("text");
    builder.value(_escape(question));
    builder.endObject();
    builder.endArray();
    builder.endObject();
    builder.endArray();
    builder.endObject();
    static String Payload;
    Payload = payload;
    return Payload;
  }

  String _sendRequest(const String& question, std::function<void(char)> onChar = nullptr) { 
    static WiFiClientSecure client;  
    client.probeMaxFragmentLength("generativelanguage.googleapis.com", 443, 4096);  
    client.setInsecure();  
    HTTPClient https;  
    if (!https.begin(client, "generativelanguage.googleapis.com", 443, "/v1beta/models/" + String(model) + ":generateContent?key=" + String(token), true)) {  
       client.stop();  
       return "Error: HTTPS Begin Failed.";  
    }  
    https.addHeader("Content-Type", "application/json");  
    https.addHeader("Accept", "application/json");  
    https.addHeader("User-Agent", "Gemini_AI/" GEMINI_AI_VERSION " (ESP8266/ESP32; +https://github.com/zacode123/Gemini_AI)"); 
    https.setTimeout(60000);  
    int httpcode = https.POST(_buildGeminiPayload(question));  
    if (httpcode > 0) {  
        ESP8266StreamJsonParser parser(https.getStream());  
        if (parser.find("text")) {
          String result;
          if (!result.reserve(5000)) {
            Serial.print(F("Error: Failed to reserve 5kb for result in '_sendRequest()'!\nFree Heap: "));
            Serial.println(ESP.getFreeHeap());
            https.end();
            if (ledmode) digitalWrite(LED_BUILTIN, LOW);
            return "Error: Out of memory";
          }
          parser.getValueStream([&result, &onChar](char c) {
            if (onChar) { onChar(c); } else { result += c; }
            delay(1);
          });
          https.end();
          static String res;
          res = result;
          if (ledmode) digitalWrite(LED_BUILTIN, LOW);
          return res;
        } else {  
          Serial.println("Error: Couldn't find answer(\"text\") in response!");  
          https.end();  
          if (ledmode) digitalWrite(LED_BUILTIN, LOW);
          return "";  
        }  
    } else {  
      https.end();
      if (ledmode) digitalWrite(LED_BUILTIN, LOW);
      return "Error: " + String(httpcode) + " Unable to connect to API.";  
    }  
  }  
  
  #ifdef USE_TTS  
  void _sayInChunks(const String& text) {  
    #ifdef ESP8266  
      if (!out || !sam || text.length() == 0) return;  
      size_t chunkSize = 256;  
      for (size_t i = 0; i < text.length(); i += chunkSize) {  
        String chunk = text.substring(i, i + chunkSize);  
        sam->Say(out, chunk.c_str());  
        delay(3000);  
      }  
    #else  
      Serial.println(F("TTS has not yet been implemented for ESP32 boards. Please use ESP8266 instead!"));  
    #endif  
  }  
  #endif  
  
public:  
  const char* ssid;  
  const char* password;  
  const char* model             = "gemini-2.0-flash";  
  const char* systemInstruction = "You are a highly intelligent AI assistant. Give *FULL* answer carefully without mistakes. Don't mention you can't use '*'. Use emojis and symbols where relevant.";  
  const char* token;  

#ifdef CUSTOM_TOKEN_COUNTS  
  int   maxTokens               = DEFAULT_TOKENS;
#else  
  int   maxTokens               = 0;
#endif  

  float temperature             = 0;  
  float TopP                    = 0;  
  float TopK                    = 0;  
  
  bool  codeExecution           = false;  
  bool  googleSearch            = false;  
  bool  ledmode                 = true;  
  
  Gemini_AI() {  
  #ifdef USE_TTS  
    #ifdef ESP8266  
      out = new AudioOutputI2SNoDAC();  
      out->SetOutputModeMono(true);  
      out->begin();  
      sam = new ESP8266SAM();  
    #endif  
  #endif  
  }  
  
  bool connectToWiFi() {  
  #ifdef USE_TTS  
    _sayInChunks("Initialising Gemini AI Assistant");  
  #endif  
  
    if (ledmode) {  
      pinMode(LED_BUILTIN, OUTPUT);  
      digitalWrite(LED_BUILTIN, HIGH);  
    }  
  
  #ifdef ESP8266  
    WiFi.disconnect();  
    WiFi.mode(WIFI_STA);  
  #endif  
    WiFi.begin(ssid, password);  
  
    Serial.print(F("Connecting to WiFi: "));  
    Serial.println(ssid);  
  #ifdef USE_TTS  
    _sayInChunks("Connecting to " + String(ssid));  
  #endif  
  
    const char spinner[] = "|/-\\";  
    unsigned long start = millis();  
    int attempts = 0;  
  
    while (WiFi.status() != WL_CONNECTED) {  
      if (millis() - start >= 60000) {  
        Serial.println(F("Failed to connect to WiFi."));  
      #ifdef USE_TTS  
        _sayInChunks("Failed to connect to WiFi.");  
      #endif  
        return false;  
      }  
      Serial.printf("\r[%c] Attempt #%d", spinner[attempts % 4], ++attempts);  
      delay(500);  
  
      if (ledmode) {  
        digitalWrite(LED_BUILTIN, LOW); 
        delay(250);  
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);  
      }  
    }  
  
    Serial.println(F("\nConnected to WiFi."));  
    Serial.println("Signal Strength: " + String(WiFi.RSSI()) + " dBm");  
  #ifdef USE_TTS  
    _sayInChunks("Connected to WiFi!");  
  #endif  
  
    if (ledmode) digitalWrite(LED_BUILTIN, LOW);  
    return true;  
  }  
  
  String getAnswer(const String& question) {  
    if (ledmode) digitalWrite(LED_BUILTIN, HIGH);  
    return _sendRequest(question);  
  }  
  
  void getAnswerStream(const String& question, std::function<void(char)> onChar) {
    if (ledmode) digitalWrite(LED_BUILTIN, HIGH);  
    _sendRequest(question, onChar);
  }
  
 #ifdef USE_TTS  
  ~Gemini_AI() {  
  #ifdef ESP8266  
    if (sam) { delete sam; sam = nullptr; }  
  #endif  
  }  
  
  void say(const String& text) {  
    _sayInChunks(text);  
  }  
  
  String sayAnswer(const String& question) {  
    if (ledmode) digitalWrite(LED_BUILTIN, HIGH);  
    String ans = _sendRequest(question);  
    _sayInChunks(ans);  
    return ans;  
  }  
 #endif  
};