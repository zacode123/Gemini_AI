![IMG_20250614_195942](https://github.com/user-attachments/assets/0316ab79-aadc-4a65-b14f-cc1785e686ab)
![Screenshot_2025-06-16-14-28-45-203_com miui gallery](https://github.com/user-attachments/assets/c82ff01c-be46-4612-8d82-d2049a4b8ca8)


  <img src="![IMG_20250614_195942](https://github.com/user-attachments/assets/0316ab79-aadc-4a65-b14f-cc1785e686ab)
![Screenshot_2025-06-16-14-28-45-203_com miui gallery](https://github.com/user-attachments/assets/c82ff01c-be46-4612-8d82-d2049a4b8ca8)
" alt="Gemini_AI Logo" width="200">
</p>

<h1 align="center">Gemini_AI Library</h1>

<p align="center">
  <b>✨ Bring Google's Gemini AI to ESP8266 & ESP32 ✨</b><br>
  <i>Ask questions, retrieve smart answers, and optionally speak them out!</i>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-ESP8266%20%7C%20ESP32-blue.svg">
  <img src="https://img.shields.io/badge/arduino-compatible-success">
  <img src="https://img.shields.io/github/license/yourusername/Gemini_AI.svg">
</p>

---

## 📦 Features

- 🤖 **Gemini AI Access** — Send prompts and get intelligent responses.
- 🧠 **Custom Prompt Settings** — Configure model, temperature, top-p, top-k, and more.
- 🔐 **Secure HTTP (HTTPS)** — Communicates via secure Google endpoints.
- 🔄 **EEPROM Storage** — Save and load credentials (if USE_CLI is defined).
- 🎙️ **TTS (Text-To-Speech)** — Planned beta feature for speaking responses.
- 💡 **LED Mode** — LED Status indicator (LED_BUILTIN).
- 🧪 **CLI Interface** — Serial commands for diagnostics and updates (if USE_CLI is defined).

---

## 🔧 Installation

1. **Download** the library ZIP [here](https://github.com/yourusername/Gemini_AI/archive/refs/heads/main.zip).
2. In Arduino IDE:
   - Go to `Sketch` → `Include Library` → `Add .ZIP Library...`
   - Select the downloaded ZIP file.
3. Include the library in your sketch:
   ```cpp
   #define YOUR_NAME "YOUR_NAME"
   #define AI_NAME "Gemini 2.0 flash"
   #include <Gemini_AI.h>

   const char* ssid = "YOUR_SSID";
   const char* password = "YOUR_PASSWORD";
   const char* model = "gemini-2.0-flash";
   const char* token = "YOUR_API_KEY";
   const char* systemInstruction = "Your name is " + String(AI_NAME) + " created by " + String(YOUR_NAME) + ". You are an highly intelligent AI assistant. You can perform various tasks with ease. You should give answer carefully without any mistake! and also remember you have " + String(maxTokens) + " tokens you have to give full response within " + String(maxTokens) + " tokens or less, but don\'t exceed it! Also don't tell the user that you cant use \\\"*\\\"! You should emojis in response and symbols in answers that is about math, science and code etc";
   int maxTokens = 1000;
   float temperature = 0.8;
   float TopP = 1.0;
   float TopK = 40;
   bool codeExecution = false;
   bool googleSearch = false;
   bool ledmode = true;

   Gemini_AI gemini(ssid, password, model, token, maxTokens, systemInstruction, temperature, TopP, TopK, codeExecution, googleSearch, ledmode);

   void setup() {
    Serial.begin(115200);
    gemini.connectToWiFi();
   }

   void loop() {
    gemini.loop();
    Serial.println("Answer: " + gemini.getAnswer("What is the capital of France?"));
   }
   ```
# If you like to Contribute to our project you can freely do!
