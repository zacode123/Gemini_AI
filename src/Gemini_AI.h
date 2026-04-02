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

#if !defined(ESP8266) && !defined(ESP32)
  #error "Gemini_AI only supports ESP8266 or ESP32 boards!"
#endif

#ifdef __cplusplus

  #define GEMINI_AI_VERSION "6.6.0"
  
  #ifdef ESP8266
    #ifndef GEMINI_SHA1_FINGERPRINT
      #define GEMINI_SHA1_FINGERPRINT "DF:A1:DB:1F:BC:5E:31:D7:F8:FE:26:E3:B9:B3:02:98:B1:C8:50:EC"
    #endif
  #endif
  
  #if defined(ESP8266)
    #define PAYLOAD_BUFFER_SIZE 1500
  #elif defined(ESP32)
    #define PAYLOAD_BUFFER_SIZE 2048
  #endif

  #if defined(ESP8266)
    #define MAX_TOKENS 1000 
    #define DEFAULT_TOKENS 500 
  #elif defined(ESP32)
    #define MAX_TOKENS 5000
    #define DEFAULT_TOKENS 3000
  #endif

  #ifdef DEBUG
    #define debug(x)      do { Serial.print(F("Gemini_AI: ")); Serial.print(x); } while(0)
    #define debugln(x)    do { Serial.print(F("Gemini_AI: ")); Serial.println(x); } while(0)
    #define debugF(x)     do { Serial.print(F("Gemini_AI: ")); Serial.print(F(x)); } while(0)
    #define debuglnF(x)   do { Serial.print(F("Gemini_AI: ")); Serial.println(F(x)); } while(0)
  #else
    #define debug(x)
    #define debugln(x)
    #define debugF(x)
    #define debuglnF(x)
  #endif
  
  #include "GeminiClient.hpp"
  #include "StreamJsonParser.hpp"
  #include "StaticJsonBuilder.hpp"

  class Gemini_AI {
  
    private:

      const char* model = "gemini-3.1-flash-lite";
      const char* systemInstruction = "You are a highly intelligent AI assistant. Use emojis and symbols where relevant.";
      const char* apiKey = nullptr;
      
      int maxTokens = DEFAULT_TOKENS;
    
      float temperature = 0;
      float TopP = 0;
      float TopK = 0;
    
      bool codeExecution = false;
      bool googleSearch = false;

      String _escape(const String& s) {
        int outlength = s.length() + (s.length() / 10);
        String out;
        if (!out.reserve(outlength)) {
          debugln("Failed to reserve" + String(outlength) + "b while escaping special charecters!");
        }
        for (char c: s) {
          switch (c) {
            case '\"': out.concat(F("\\\"")); break;
            case '\\': out.concat(F("\\\\")); break;
            case '\b': out.concat(F("\\b")); break;
            case '\f': out.concat(F("\\f")); break;
            case '\n': out.concat(F("\\n")); break;
            case '\r': out.concat(F("\\r")); break;
            case '\t': out.concat(F("\\t")); break;
            default: out += c; break;
          }
        }
        return out;
      }

      String _buildGeminiPayload(const String& question) {
        int maxtokens = std::min(maxTokens, MAX_TOKENS);
        char payload[PAYLOAD_BUFFER_SIZE];
        StaticJsonBuilder builder(payload, PAYLOAD_BUFFER_SIZE, false);
        builder.beginObject();
        if (googleSearch || codeExecution) {
          builder.key("tools");
          builder.beginArray();
          builder.beginObject();
          if (googleSearch && codeExecution) {
            builder.key("googleSearch");
            builder.beginObject();
            builder.endObject();
            builder.key("codeExecution");
            builder.beginObject();
            builder.endObject();
          } else {
            if (googleSearch) {
              builder.key("googleSearch");
              builder.beginObject();
              builder.endObject();
            } else if (codeExecution) {
              builder.key("codeExecution");
              builder.beginObject();
              builder.endObject();
            }
          }
          builder.endObject();
          builder.endArray();
        }
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
          builder.value(_escape(String(systemInstruction) + "Give *FULL* answer carefully without mistakes. Don't mention you can't use '*'"));
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
        return String(payload);
      }
    
      String _sendRequest(const String& question, std::function < void(char) > onChar = nullptr) {
        if (WiFi.status() != WL_CONNECTED) {
          debuglnF("WiFi not connected!");
          return "";
        }
        GeminiClient client;
        if (!client.begin(String(model), String(apiKey))) {
          client.end();
          debuglnF("GeminiClient Begin Failed.");
          return "";
        }
        int httpcode = client.POST(_buildGeminiPayload(question));
        if (httpcode > 0) {
          if (httpcode == 200 || httpcode == 301) {
            StreamJsonParser parser(client.getStream());
            if (parser.find("text")) {
              String result;
              #if defined(ESP8266)
                if (!result.reserve(512)) {
                  debugF("Failed to reserve 512 bytes for result.!\nFree Heap: ");
              #elif defined(ESP32)
                if (!result.reserve(4096)) {
                  debugF("Failed to reserve 4kbs for result.!\nFree Heap: ");  
              #endif
                debugln(ESP.getFreeHeap());
                client.end();
                return "";
              }
              parser.getValueStream([&result, &onChar](char c) {
                if (onChar) {
                  onChar(c);
                } else {
                  result += c;
                }
                delay(1);
              });
              client.end();
              return result;
            } else {
              debuglnF("Couldn't find answer(\"text\") in response!");
              client.end();
              return "";
            }
          } else {
            WiFiClient &stream = client.getStream();
            debuglnF("ERROR : \n");
            while (client.connected() || stream.available()) {
              if (stream.available()) {
                Serial.write(stream.read());
              }
              delay(50);
            }
            debugln();
            return "";
          }
        } else {
          client.end();
          debugln("Payload POST Error: " + String(httpcode) + ", " + client.errorToString(httpcode));
          return "";
        }
      }

    public:

      Gemini_AI() {}
      ~Gemini_AI() {}

      bool begin() {
        if (WiFi.status() != WL_CONNECTED) {
          debuglnF("WiFi not connected!");
          return false;
        }
        if (!apiKey) {
          debuglnF("API key not set!");
          return false;
        }
        return true;
      }
    
      void useModel(const char* m) {
        model = m;
      }
    
      void setSystemInstruction(const char* instr) {
        systemInstruction = instr;
      }
    
      void setApiKey(const char* t) {
        apiKey = t; 
      }
    
      void setMaxTokens(int t) {
        maxTokens = t;
      }
    
      void setTemperature(float t) {
        temperature = t;
      }
    
      void setTopP(float p) {
        TopP = p;
      }
    
      void setTopK(float k) {
        TopK = k;
      }
    
      void enableCodeExecution()  { 
        codeExecution = true; 
      }
    
      void disableCodeExecution() {
        codeExecution = false; 
      }
    
      void enableGoogleSearch()  { 
        googleSearch = true;  
      }
    
      void disableGoogleSearch() { 
        googleSearch = false;
      }
      
      const char* getModel() {
        return model;
      }
    
      const char* getSystemInstruction() {
        return systemInstruction;
      }
    
      const char* getApiKey() {
        return apiKey;
      }
    
      int getMaxTokens() {
        return maxTokens;
      }
    
      float getTemperature() {
        return temperature;
      }
    
      float getTopP() {
        return TopP;
      }
    
      float getTopK() {
        return TopK;
      }
    
      bool getCodeExecution() {
        return codeExecution; 
      }
    
      bool getGoogleSearch() { 
        return googleSearch;
      }
    
      String getAnswer(const String& question) {
        return _sendRequest(question);
      }

      void getAnswerStream(const String& question, std::function < void(char) > onChar) {
        _sendRequest(question, onChar);
      }
  };
#else
  #error "Gemini_AI requires a C++ compiler. Please rename your file to .cpp or .cc"
#endif
