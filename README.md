# ✨ Gemini AI Library ✨

<p align="center">
  <b>Bring Google's Gemini AI to low-powered microcontrollers, ESP8266 & ESP32</b><br>
  <i>Ask questions and get intelligent responses right from your microcontroller!</i>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-ESP8266%20%7C%20ESP32-blue.svg">
  <img src="https://img.shields.io/badge/arduino-compatible-success">
  <img src="https://img.shields.io/github/license/zacode123/Gemini_AI.svg">
</p>

---

## 📦 Features

- 🤖 **Gemini AI Access** — Send prompts and receive smart responses ultra fast.
- 🧠 **Custom Prompt Settings** — Configure Model, Temperature, TopP, TopK, and much more.
- 🔐 **Secure HTTP (HTTPS)** — Communicates safely with Google endpoints.
- 💡 **Memory Efficient** — Stream-based JSON parsing and static JSON building minimize RAM usage.
- ⚡ **Real-Time Streaming** — Stream responses as they arrive for chat-like interactions.

---

## 🚀 What's New (v6.6.0)

### 🧩 Modular Design
- Separate `.hpp` files for clean structure:
  - `Gemini_AI.h` — main interface
  - `Gemini_Cert.h` — Google Root CA certificate used by Gemini AI.
  - `GeminiClient.hpp` — HTTPS client helper
  - `StaticJsonBuilder.hpp` — stack-based JSON serialization
  - `StreamJsonParser.hpp` — stream-based JSON parser
- Easier maintenance and future enhancements.

### ⚡ Memory Efficiency
- Stream reading avoids full response buffering.
- Max tokens limit ensures stability:
  - ESP8266: default 500 tokens, max 1000
  - ESP32: default 3000 tokens, max 5000
- Use `setMaxTokens(0)`, you don't want to specify a fixed no. of max tokens.

### 🛠️ Static JSON Builder
- Uses only stack and static memory—no malloc, new, or String—perfect for low-RAM environments like ESP8266.
- Ultra-fast serialization, lightweight and super fast JSON building.
- Supports deeply nested objects and arrays. Handles complex JSON structures with configurable maximum nesting depth.
- Payload is generated using this library.

### 🧠 Smart JSON Handling
- `getAnswerStream("Question", Callback)` streams AI responses in real-time.
- Handles deeply nested responses up to 5000 tokens.

### 🔐 Secure HTTPS Connections
- Works with ESP8266 and ESP32 secure clients.
- Supports CA certificate or SHA1 fingerprint fallback.
- Keeps your queries and API keys safe.

---

## 🔧 Installation

1. Download the library ZIP from GitHub.
2. In Arduino IDE:
   - `Sketch → Include Library → Add .ZIP Library...`
   - Select the downloaded ZIP.
3. Include in your sketch:
```cpp
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif
#include <Gemini_AI.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* pass = "YOUR_WIFI_PASSWORD";

Gemini_AI gemini;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, pass);
  Serial.print(F("Connecting to WiFi"));
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("."));
    delay(500);
  }
  Serial.println(F("Connected!"));

  gemini.begin();
  gemini.setApiKey("YOUR_API_KEY");

  String answer = gemini.getAnswer("What is the capital of France?");
  Serial.println("Answer: " + answer);
}

void loop() {}
```

---

⭐ Notes

• No TTS or LED indicator in this version.
• Connect to WiFi manually before using the library.
• Defining the DEBUG macro enables verbose library logs.
• Default System Instruction is `You are a highly intelligent AI assistant. Use emojis and symbols where relevant.`. If you want disable System Instruction, just use `setSystemInstruction("")`. I would not recomend you to disable instruction as it can send a lot of asterisks in response.
• Modular design allows easy maintenance and future improvements.

---

💡 Why Use Gemini AI Library?

• Real-time streaming → interactive AI experience.
• Memory safe → no crashes on low-RAM devices.
• Modular & maintainable → future-proof your code.
• Secure communication → HTTPS encryption ensures safety.

---

📖 Example: Streaming Responses

gemini.getAnswerStream("Tell me a story", [](String chunk){
    Serial.print(chunk);
});

Streams data in real-time, perfect for chat apps or live AI feedback.

---

🔗 Contribute & Support

Love this library? Give it a ⭐ on GitHub!

Found a bug or have an idea? Open an issue or submit a pull request.

Your contributions make Gemini AI even smarter on ESP boards.

---

<p align="center">
  Made with ❤️ for ESP8266 & ESP32 developers
</p>

## *🛸 Like this project? Show some love by giving it a ⭐️!*