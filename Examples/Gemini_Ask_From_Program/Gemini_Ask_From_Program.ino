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
#include <credentials.h>

// Create an instance of the Gemini_AI class
Gemini_AI gemini(ssid, password, model, token, maxTokens, systemInstruction, temperature, TopP, TopK, codeExecution, googleSearch, ledmode);

void setup() {
  Serial.begin(115200);
  if (gemini.connectToWiFi()) {
  Serial.println("Connected to ");
  Serial.println(ssid);
  // Query the Gemini API
  String question = "What is the capital of France?";
  Serial.println("Question: " + question);
  Serial.println("Answer: " + gemini.getAnswer(question));
  }
}
void loop() {}
