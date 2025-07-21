![Screenshot_2025-06-16-14-28-45-203_com miui gallery](https://github.com/user-attachments/assets/c82ff01c-be46-4612-8d82-d2049a4b8ca8)

<h1 align="center">Gemini AI Library</h1>

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
- 💡 **LED Indicator** — LED Status indicator (LED_BUILTIN).

---

## 🚀 What's New (v6.5.0)

### 🗂️ File management
- All related source files are now modularized into .hpp headers.
- Each header is now self-contained with minimal external dependencies.
- Only necessary components are compiled into the final firmware.

### 🛠️ Static JSON Builder (New `ESP8266StaticJsonBuilder.hpp`)
- Uses only stack and static memory—no malloc, new, or String—perfect for low-RAM environments like ESP8266.
- Ultra-fast serialization and ightweight and fast JSON building into a user-supplied char[] buffer.
- Supports deeply nested objects and arrays. Handles complex JSON structures with configurable maximum nesting depth.
- Payload is generated using this library.

### 🔄 Stream-Based JSON Parsing (New `ESP8266StreamJsonParser.hpp`)
- Introduced a **custom, lightweight JSON parser** built specifically for ESP8266 with low memory.
- Parses deeply nested streamed JSON directly from `WiFiClient`, **without loading entire strings into memory**.
- Ideal for Gemini AI's large responses—ensures stable parsing on 80KB RAM boards.
- Added a new function called `getAnswerStream` which streams each char from stream. For use see the `Gemini_Get_Answer_Stream` example folder.
- Now it can parse responses upto 5000 tokens or greater.

### 🧠 Gemini_AI Memory Efficiency Boost
- Reduced dynamic allocations by replacing full response buffering with **progressive stream reading**.
- Memory usage optimized for **real-time inference** and **chunked text extraction**.
- Internal `_extractContent()` now uses `ESP8266Json` to extract `"text"` efficiently.
- Added a limit of 5000 `maxTokens` for **normal use** and 400 `maxTokens` for `USE_TTS`, to optimize memory issues. `maxTokens` greater than its limit will fallback to the limit.

### 📦 Modular `Gemini_AI.hpp` Improvements
- Cleaner separation of parsing logic and request handling.
- Enables future drop-in replacements or alternative parsers without modifying core logic.

---

## 💡 LED Indicator

#### **By default the led indicator is on but you can turn it off by using `gemini.ledmode=false`.**

### Indications :

- LED blinking means it is trying to connect to WiFi.
- LED turned on means it's ready for asking question.
- LED turned off means it's processing the question.

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

### 🛸 Like this project? Show some love by giving it a ⭐️!