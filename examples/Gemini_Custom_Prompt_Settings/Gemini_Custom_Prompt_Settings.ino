/* ** Gemini_Custom_Prompt_Settings.ino **

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
  
  // Prompt Configurations(Settings)
  gemini.useModel("gemini-3.0-flash-lite");
  gemini.setSystemInstruction("You are a highly intelligent AI assistant. Give *FULL* answer carefully without mistakes. Don't mention you can't use '*'. Use emojis and symbols where relevant.");
  gemini.setMaxTokens(500);
  gemini.setTemperature(0.8f);
  gemini.setTopP(1.0f);
  gemini.setTopK(40.0f);
  gemini.disableCodeExecution();
  gemini.disableGoogleSearch();

  gemini.begin();
  
    // Query the Gemini API
  String question = "What is the capital of France?";
  Serial.println("Question: " + question);
  Serial.println("Answer: " + gemini.getAnswer(question));
}
void loop() {}
