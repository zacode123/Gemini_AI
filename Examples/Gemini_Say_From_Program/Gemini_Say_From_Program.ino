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

// it will basically tell a story.
String question = "Tell me a long English story. Always tell the full story and don't tell other stuff except story!";
String response;

// Create an instance of the Gemini_AI class.
Gemini_AI gemini;

void setup() {
    Serial.begin(115200);
    gemini.ssid     = "YOUR_SSID";
    gemini.password = "YOUR_PASSWORD";
    gemini.token    = "YOUR_API_KEY";
    gemini.connectToWiFi();
    gemini.say("A I will now tell some stories in a delay of 5 seconds. Let's get started!");   
    delay(3000);
}

void loop(){
    response = gemini.sayAnswer(question);
    Serial.println("Response:");
    Serial.println("        " + response);
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    delay(5000);
}