![Screenshot_2025-06-16-14-28-45-203_com miui gallery](https://github.com/user-attachments/assets/c82ff01c-be46-4612-8d82-d2049a4b8ca8)

<h1 align="center">Gemini_AI Library</h1>

<p align="center">
  <b>✨ Bring Google's Gemini AI to ESP8266 & ESP32 ✨</b><br>
  <i>Ask questions, retrieve smart answers, and optionally speak them out!</i>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-ESP8266%20%7C%20ESP32-blue.svg">
  <img src="https://img.shields.io/badge/arduino-compatible-success">
  <img src="https://img.shields.io/github/license/zacode123/Gemini_AI.svg">
</p>

---

## 📦 Features

- 🤖 **Gemini AI Access** — Send prompts and get intelligent responses.
- 🧠 **Custom Prompt Settings** — Configure model, temperature, top-p, top-k, and more.
- 🔐 **Secure HTTP (HTTPS)** — Communicates via secure Google endpoints.
- 🎙️ **TTS (Text-To-Speech)** — Planned beta feature for speaking responses for ESP8266 only.
- 💡 **LED Mode** — LED Status indicator (LED_BUILTIN).

---

## 🚀 What's New (6.4.0)

- ✨ **Added support for ESP32** — expanded compatibility and performance improvements.
- ✨ **Performance Upgraded** — Now it can handle large responses without issues.
- ✨ **Json parser** — Removed ArduinoJson by adding custom text parser from json for memory improvements (may need to be changed if api's json structure changes).

---

## 🔧 Installation

1. **Download** the library ZIP [here](https://github.com/zacode123/Gemini_AI/archive/refs/heads/main.zip).
2. In Arduino IDE:
   - Go to `Sketch` → `Include Library` → `Add .ZIP Library...`
   - Select the downloaded ZIP file.
3. Include the library in your sketch:
   ```cpp
   #include <Gemini_AI.h>
   
   Gemini_AI gemini;
   
   void setup() {
    Serial.begin(115200);
    gemini.ssid     = "YOUR_SSID";
    gemini.password = "YOUR_PASSWORD";
    gemini.token    = "YOUR_API_KEY";
    gemini.connectToWiFi();
   }

   void loop() {
    Serial.println("Answer: " + gemini.getAnswer("What is the capital of France?"));
   }
   ```

---

## Compilation 

- **Precompiled bin files are also added for esp8266 only** — Go to `Examples` → `bin` to get the bin files.
- **For Arduino IDE** — See the Installation part.

---

# ✨ Contribution

 If you like to Contribute to our project you can freely do!❤️
