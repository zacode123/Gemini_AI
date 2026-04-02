#pragma once

#define GEMINI_AI_VERSION "6.6.0"

#include <Arduino.h>
#include "Google_ROOTCa.h"

#ifdef ESP8266
  #include <memory>
  #include <WiFiClientSecure.h>
  #define SECURE_CLIENT WiFiClientSecure
#elif defined(ESP32)
  #include <NetworkClientSecure.h>
  #define SECURE_CLIENT NetworkClientSecure
#endif

#define HTTPC_ERROR_CONNECTION_REFUSED  (-1)
#define HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
#define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
#define HTTPC_ERROR_NOT_CONNECTED       (-4)
#define HTTPC_ERROR_CONNECTION_LOST     (-5)
#define HTTPC_ERROR_NO_HTTP_SERVER      (-7)

#ifdef DEBUG
  #define debug(x)      do { Serial.print(F("GeminiClient: ")); Serial.print(x); } while(0)
  #define debugln(x)    do { Serial.print(F("GeminiClient: ")); Serial.println(x); } while(0)
  #define debugF(x)     do { Serial.print(F("GeminiClient: ")); Serial.print(F(x)); } while(0)
  #define debuglnF(x)   do { Serial.print(F("GeminiClient: ")); Serial.println(F(x)); } while(0)
#else
  #define debug(x)
  #define debugln(x)
  #define debugF(x)
  #define debuglnF(x)
#endif

class GeminiClient {

  public:
    GeminiClient() {}
    ~GeminiClient() {
      end();
    }

    bool begin(const String& model, const String& apiKey) {
      clear();
      _model = model;
      _apiKey = apiKey;
      #ifdef ESP8266
        if (!_client.probeMaxFragmentLength("generativelanguage.googleapis.com", 443, 4096)) {
          debuglnF("Failed to set probeMaxFragmentLength");
        }
        if (!_client.setCACert_P(google_root_ca)) {
          debuglnF("CA cert failed. Trying SHA1 fingerprint...");
          if (!_client.setFingerprint(GEMINI_SHA1_FINGERPRINT)) {
            debuglnF("Fingerprint failed. Falling back to insecure.");
            _client.setInsecure();
          }
        } else {
          debuglnF("Using CA certificate.");
        }
      #elif defined(ESP32)
        _client.setBufferSizes(4096, 4096);
        if (!_client.setCACert(google_root_ca)) {
          debuglnF("CA cert failed. Falling back to insecure.");
          _client.setInsecure();
        } else {
          debuglnF("Using CA certificate.");
        }
      #endif
      return true;
    }

    void end() {
      if (_client.connected()) {
        _client.stop();
      }
      clear();
    }

    bool connected() {
      return (_client.connected() || _client.available() > 0);
    }

    int POST(const String& payload) {
      return sendRequest("POST", (uint8_t*)payload.c_str(), payload.length());
    }

    SECURE_CLIENT &getStream() {
      return _client;
    }

    static String errorToString(int error) {
      switch (error) {
        case HTTPC_ERROR_CONNECTION_REFUSED: return F("connection refused");
        case HTTPC_ERROR_SEND_HEADER_FAILED: return F("send header failed");
        case HTTPC_ERROR_SEND_PAYLOAD_FAILED: return F("send payload failed");
        case HTTPC_ERROR_NOT_CONNECTED: return F("not connected");
        case HTTPC_ERROR_CONNECTION_LOST: return F("connection lost");
        case HTTPC_ERROR_NO_HTTP_SERVER: return F("no HTTP server");
        default: return String(error);
      }
    }

  private:
    void clear() {
      _returnCode = 0;
      _size = -1;
    }

    bool connect() {
      if (connected()) {
        while (_client.available() > 0) {
          _client.read();
        }
        return true;
      }
      debuglnF("Connecting to generativelanguage.googleapis.com...");
      #ifdef ESP32
        if (!_client.connect("generativelanguage.googleapis.com", 443, _connectTimeout)) {
          debuglnF("Connection failed!");
          return false;
        }
      #else
        _client.setTimeout(_tcpTimeout);
        if (!_client.connect("generativelanguage.googleapis.com", 443)) {
          debuglnF("Connection failed!");
          return false;
        }
      #endif
      debuglnF("Connected successfully.");
      return connected();
    }

    bool sendHeader(const char *type, size_t payloadSize) {
      if (!connected()) {
        return false;
      }
      String header = String(type) + " /v1beta/models/" + _model + ":generateContent HTTP/1.1\r\n";
      header += "Host: generativelanguage.googleapis.com\r\n";
      header += "User-Agent: Gemini_AI/" GEMINI_AI_VERSION "\r\n";
      header += "Connection: close\r\n";
      header += "Content-Type: application/json\r\n";
      header += "Accept: application/json\r\n";
      header += "X-goog-api-key: " + _apiKey + "\r\n";
      header += "Content-Length: " + String(payloadSize) + "\r\n\r\n";

      if (_client.write((const uint8_t*)header.c_str(), header.length()) != header.length()) {
        debuglnF("Header send failed.");
        return false;
      }
      return true;
    }

    int handleHeaderResponse() {
      if (!connected()) {
        debuglnF("Client not connected while handling headers!");
        return HTTPC_ERROR_NOT_CONNECTED;
      }
      _returnCode = 0;
      bool firstLine = true;
      unsigned long start = millis();
      while (connected()) {
        if (_client.available() > 0) {
          String line = _client.readStringUntil('\n');
          line.trim();
          if (firstLine) {
            firstLine = false;
            int codePos = line.indexOf(' ') + 1;
            int codeEnd = line.indexOf(' ', codePos);
            if (codePos > 0 && codeEnd > codePos) {
              _returnCode = line.substring(codePos, codeEnd).toInt();
              debugF("HTTP Status Code: ");
              debugln(_returnCode);
            } else {
              debuglnF("Failed to parse HTTP status code!");
              _returnCode = 0;
            }
          } else if (line.startsWith("Content-Length:")) {
            _size = line.substring(15).toInt();
            debugF("Content-Length: ");
            debugln(_size);
          }
          if (line == "") {
            if (_returnCode > 0) {
              debuglnF("End of headers reached.");
              return _returnCode;
            } else {
              debuglnF("No valid HTTP status code received.");
              return HTTPC_ERROR_NO_HTTP_SERVER;
            }
          }
        }
        if (millis() - start > _tcpTimeout) {
          debuglnF("Timeout while reading headers!");
          break;
        }
        yield();
      }
      debuglnF("Connection lost before full headers received.");
      return HTTPC_ERROR_CONNECTION_LOST;
    }

    int sendRequest(const char *type, uint8_t *payload, size_t size) {
      if (!connect())
        return HTTPC_ERROR_CONNECTION_REFUSED;
      }
      if (!sendHeader(type, size)) {
        return HTTPC_ERROR_SEND_HEADER_FAILED;
      }
      if (size > 0) {
        if (_client.write(payload, size) != size) {
          debuglnF("Payload send failed.");
          return HTTPC_ERROR_SEND_PAYLOAD_FAILED;
        }
      }
      return handleHeaderResponse();
    }

    SECURE_CLIENT _client;
    String _model;
    String _apiKey;
    uint16_t _tcpTimeout = 5000;
    int32_t _connectTimeout = 10000;
    int _returnCode = 0;
    int _size = -1;
};
