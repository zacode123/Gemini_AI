/* ** Gemini_Get_Answer_From_Serial.ino **

MIT License

Copyright (c) 2025 zacode123

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

*/
  
#ifndef STASSID
  #define STASSID "YOUR_SSID"
  #define STAPSK "YOUR_PASSWORD"
#endif

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#define DEBUG
#include <Gemini_AI.h>

const char* ssid = STASSID;
const char* pass = STAPSK;

// Create an instance of the Gemini_AI class.
Gemini_AI gemini;

void setup() {
  Serial.begin(115200);
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print(F("Connecting to WiFi "));
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("."));
    delay(700);
  }
  Serial.println(F("Connected to WiFi!"));
  gemini.setApiKey("YOUR_API_KEY");
  gemini.begin();
  Serial.print(F("\t\t\t\t\t\tGemini AI assistant is ready!\n>> "));
}

void loop() {
 while (Serial.available()) {    
    String input = Serial.readStringUntil('\n');    
    input.trim();    
  if (input.length()) {    
    Serial.println(input);   
    printStream(gemini.getAnswer(input)); // For printing large responses from gemini.
  } 
 }
}

void printStream(const String &input) { // Helper function.
  if (input.length()) {    
    size_t totalLen = input.length();    
    for (size_t i = 0; i < totalLen; i += 64) {    
      String chunk = input.substring(i, i + 64);    
      Serial.print(chunk);    
      delay(100);    
      yield();    
    }    
      Serial.print(F("\n>> "));      
   }
}