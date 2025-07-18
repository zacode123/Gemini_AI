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
  
#include <Gemini_AI.h>

// Create an instance of the Gemini_AI class.
Gemini_AI gemini;

void setup() {
  Serial.begin(115200);
  gemini.ssid     = "YOUR_SSID";
  gemini.password = "YOUR_PASSWORD";
  gemini.token    = "YOUR_API_KEY";
 if (gemini.connectToWiFi()) {  // DO NOT REMOVE! library's internal connect to wifi function.
  Serial.println(F("Gemini AI assistant is ready!\n>> "));
 } else {
  ESP.restart();
 }
}

void loop() {
 while (Serial.available()) {    
    String input = Serial.readStringUntil('\n');    
    input.trim();    
  if (input.length()) {    
    Serial.print(F(">> "));    
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
