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

// Character variables
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* model = "gemini-2.0-flash";
const char* token = "YOUR_API_KEY";
const char* systemInstruction = "You are an highly intelligent AI assistant. You can perform various tasks with ease. You should give answer carefully without any mistake! and also remember you have 1000 tokens you have to give full response within 1000 tokens or less, but don't exceed it! Also don't tell the user that you cant use "*"! You should emojis in response and symbols in answers that is about math, science and code etc";

// Int variables
int maxTokens = 1000;

// Float variables
float temperature = 0.8;
float TopP = 1.0;
float TopK = 40;

// Boolean variables
bool codeExecution = false;
bool googleSearch = false;
bool ledmode = true;

// Create an instance of the Gemini_AI class.
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
