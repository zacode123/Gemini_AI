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

  // Prompt Configurations(Settings)
  gemini.ssid              = "YOUR_SSID";
  gemini.password          = "YOUR_PASSWORD";
  gemini.model             = "gemini-2.0-flash";
  gemini.systemInstruction = "You are a highly intelligent AI assistant. Give *FULL* answer carefully without mistakes. Don't mention you can't use '*'. Use emojis and symbols where relevant.";
  gemini.token             = "YOUR_API_KEY";
  gemini.maxTokens         = 500;
  gemini.temperature       = 0.8;
  gemini.TopP              = 1.0;
  gemini.TopK              = 40;
  gemini.codeExecution     = false;
  gemini.googleSearch      = false;
  gemini.ledmode           = true;

  if (gemini.connectToWiFi()) {
    // Query the Gemini API
    String question = "What is the capital of France?";
    Serial.println("Question: " + question);
    Serial.println("Answer: " + gemini.getAnswer(question));
  }
}
void loop() {}