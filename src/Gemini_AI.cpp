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

#include "Gemini_AI.h"        
    
Gemini_AI::Gemini_AI() {
#ifdef ESP8266
  out = new AudioOutputI2SNoDAC();    
  out->SetOutputModeMono(true);    
  out->begin();    
  sam = new ESP8266SAM(); 
#endif   
}    
    
Gemini_AI::~Gemini_AI() {    
#ifdef ESP8266
  if (sam) { delete sam; sam = nullptr; } 
#endif
}    
    
bool Gemini_AI::connectToWiFi() {       
  _sayInChunks("Initialising Gemini AI Assistant");    
  delay(300);    
  if (ledmode) {    
    pinMode(LED_BUILTIN, OUTPUT);    
    digitalWrite(LED_BUILTIN, HIGH);    
  }    
#ifdef ESP8266    
  WiFi.disconnect();    
  WiFi.mode(WIFI_STA);    
  WiFi.begin(ssid, password);    
#else    
  WiFi.begin(ssid, password);    
#endif    
    
  Serial.print(F("Connecting to WiFi: "));    
  Serial.println(ssid);    
  _sayInChunks("Connecting to " + String(ssid));    
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
   if (ledmode) {    
      digitalWrite(LED_BUILTIN, LOW);    
      delay(250);    
      digitalWrite(LED_BUILTIN, HIGH);    
      delay(250);    
    }    
  }    
  Serial.println(F("\nConnected to WiFi."));    
  Serial.println("Signal Strength: " + String(WiFi.RSSI()) + " dBm");    
  _sayInChunks("Connected to WiFi!");    
  if (ledmode) digitalWrite(LED_BUILTIN, LOW);    
  Serial.print(F("IP Address: "));    
  Serial.println(WiFi.localIP());    
  _sayInChunks("IP Address: " + WiFi.localIP().toString());    
  return true;    
}    
    
String Gemini_AI::_sendRequest(const String& question) {
  WiFiClientSecure client; 
  client.setBufferSizes(4096, 1024);   
  client.setInsecure();    
    
  HTTPClient http;    
  String url = "https://generativelanguage.googleapis.com/v1beta/models/"    
               + String(model) + ":generateContent?key=" + String(token);    
  yield();    
if (!http.begin(client, url)) return "Error: HTTP Begin Failed.";    
  http.setTimeout(15000);    
  http.addHeader("Content-Type", "application/json");    
  http.addHeader("Accept", "application/json");    
    
  bool isImageModel = String(model).indexOf("image-generation") >= 0;    
  String modalities = isImageModel    
    ? "[\"IMAGE\", \"TEXT\"]"    
    : "[\"TEXT\"]";    
    
  String tools = "[";    
  if (googleSearch) tools += "{ \"googleSearch\": {} },";    
  if (codeExecution) tools += "{ \"codeExecution\": {} },";    
  if (tools.endsWith(",")) tools.remove(tools.length()-1);    
  tools += "]";    
    
  String payload = "{";    
  payload += "\"tools\":" + tools + ",";    
  payload += "\"generationConfig\":{";    
  payload += "\"temperature\":" + String(temperature, 2) + ",";    
  payload += "\"topP\":" + String(TopP, 2) + ",";    
  payload += "\"topK\":" + String(TopK, 2) + ",";    
  payload += "\"maxOutputTokens\":" + String(maxTokens) + ",";    
  payload += "\"responseModalities\":" + modalities + ",";    
  payload += "\"responseMimeType\":\"text/plain\"";    
  payload += "},";    
  payload += "\"systemInstruction\":{";    
  payload += "\"parts\":[{\"text\":\"" + _escape(String(systemInstruction), true) + "\"}]";    
  payload += "},";    
  payload += "\"contents\":[{";    
  payload += "\"role\":\"user\",";    
  payload += "\"parts\":[{\"text\":\"" + _escape(question, true) + "\"}]";    
  payload += "}]";    
  payload += "}";    
  yield();    
  int code = http.POST(payload);    
  yield();    
  if (code > 0) {    
    response = http.getString();    
    http.end();    
    yield();      
    return _extractContent(response);    
  }    
  http.end();    
  yield();    
  return "Error: Unable to connect to API.";    
}    
    
void Gemini_AI::_sayInChunks(const String& text) {    
#ifdef ESP8266
  if (!out || !sam || text.length() == 0) return;   
   size_t chunkSize = 256;
   for (size_t i = 0; i < text.length(); i += chunkSize) {    
    String chunk = text.substring(i, i + chunkSize);    
    sam->Say(out, chunk.c_str());    
    delay(3000);    
   }  
#endif
   Serial.println(F("TTS is not yet implemented for ESP32 boards. Please use ESP8266 instead!")); 
}    
    
void Gemini_AI::say(const String& text) {
   _sayInChunks(text);
}

String Gemini_AI::getAnswer(const String& question) {      
  return _sendRequest(question);    
}    
    
String Gemini_AI::sayAnswer(const String& question) {      
  String ans = _sendRequest(question);    
  _sayInChunks(ans);    
  return ans;    
}         
    
String Gemini_AI::_extractContent(const String& response) {    
  int lineStart = response.indexOf("\"text\":");    
if (lineStart == -1) return "No content line found.\n\n\t\tRESPONSE ====>\n" + response;    
  int lineEnd = response.indexOf('\n', lineStart);    
if (lineEnd == -1) lineEnd = response.length();    
  String line = response.substring(lineStart, lineEnd);    
  int quoteStart = line.indexOf('\"', line.indexOf(":"));    
if (quoteStart == -1) return "Content quote not found.\n\nRESPONSE ====>\n\t" + response;    
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
       content = _decodeUnicode(_escape(content, false));    
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

String Gemini_AI::_escape(const String& s, bool encode) {
  String out;
 if (encode) {
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    switch (c) {
      case '\"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (c >= 0 && c <= 0x1F) {
          char buf[7];
          sprintf(buf, "\\u%04x", c);
          out += buf;
        } else {
          out += c;
        }
    }
  }
 } else {
     String copy = s;
     copy.replace("\\n", "\n");
     copy.replace("\\r", "\r");
     copy.replace("\\t", "\t");
     copy.replace("\\f", "\f");
     copy.replace("\\b", "\b");
     copy.replace("\\\"", "\"");
     copy.replace("\\\\", "\\");
     out = copy;
  }
  return out;
} 