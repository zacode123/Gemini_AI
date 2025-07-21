/*
 * ESP8266JsonStreamParser.hpp - A simple and memory-efficient JSON parser for ESP8266.
 *
 * This library is designed to parse JSON data from a stream (like a file)
 * without loading the entire file into memory. This makes it ideal for
 * resource-constrained devices like the ESP8266.
 *
 * It's a rule-based parser, meaning it looks for specific patterns (keys)
 * in the JSON and extracts the corresponding values. This version adds
 * robustness for deeply nested and streamed JSON, such as those returned
 * by Gemini AI APIs.
 *
 * MIT License
 *
 * Copyright (c) 2025 zacode123
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * Created by zacode123, 16-07-2025
 * Version 2.5.0 (Corrected Recursive Search Logic)
 *
 * CHANGELOG:
 * - v2.5.0 (19-07-2025):
 * - CRITICAL FIX: Rewrote the _findKeyRecursive function to correctly search
 * inside nested objects and arrays instead of incorrectly skipping them.
 * - The previous logic would skip entire arrays/objects if the top-level key
 * didn't match, causing it to miss nested keys like "text".
 * - The new logic correctly descends into the JSON structure, ensuring all
 * nodes are checked. This finally resolves the "key not found" error.
 */

#pragma once

#include <Arduino.h>
#include <Stream.h>
#include <WString.h>
#include <ctype.h>
#include <functional>

class ESP8266StreamJsonParser {
public:
  ESP8266StreamJsonParser(Stream &stream) : _stream(stream), _peek('\0') {}

  bool find(const char *key) {
    _peek = '\0';
    _skipWhitespace();
    while (_stream.available()) {
      char c = _stream.peek();
      if (c == '{' || c == '[') break;
      _stream.read();
    }
    char first_char = _stream.peek();
    if (first_char != '{' && first_char != '[') {
      return false;
    }
    return _findKeyRecursive(key);
  }
  
  void getValueStream(std::function<void(char)> onChar) {
    _skipWhitespace();
    char c = _stream.peek();
    if (c == '"') {
      _read();
      while (true) {
        while (!_stream.available()) delay(1);
        char ch = _read();
        if (ch == '"') break;
        if (ch == '\\') {
          while (!_stream.available()) delay(1);
            char next = _read();
            switch (next) {
              case '"': onChar('"'); break;
              case '\\': onChar('\\'); break;
              case '/': onChar('/'); break;
              case 'b': onChar('\b'); break;
              case 'f': onChar('\f'); break;
              case 'n': onChar('\n'); break;
              case 'r': onChar('\r'); break;
              case 't': onChar('\t'); break;
              case 'u': {
                 String hex1;
                 if (!hex1.reserve(5)) {
                    Serial.println(F("Error: Failed to reserve 5b for hex1 in 'getValueStream()'!"));
                 }
                 for (int i = 0; i < 4; ++i) {
                    hex1 += (char)_read();
                 }
                 uint16_t cp1 = strtol(hex1.c_str(), nullptr, 16);
                 if (cp1 >= 0xD800 && cp1 <= 0xDBFF && _stream.peek() == '\\') {
                    _read();
                    if (_stream.peek() == 'u') {
                        _read();
                        String hex2;
                        if (!hex2.reserve(5)) {
                           Serial.println(F("Error: Failed to reserve 5b for hex2 in 'getValueStream()'!"));
                        }
                        for (int i = 0; i < 4; ++i) {
                           hex2 += (char)_read();
                        }
                       uint16_t cp2 = strtol(hex2.c_str(), nullptr, 16);
                       if (cp2 >= 0xDC00 && cp2 <= 0xDFFF) {
                           uint32_t full = 0x10000 + ((cp1 - 0xD800) << 10) + (cp2 - 0xDC00);
                           onChar((char)(0xF0 | ((full >> 18) & 0x07)));
                           onChar((char)(0x80 | ((full >> 12) & 0x3F)));
                           onChar((char)(0x80 | ((full >> 6) & 0x3F)));
                           onChar((char)(0x80 | (full & 0x3F)));
                           break;
                       }
                    }
                }
                if (cp1 < 0x80) {
                   onChar((char)cp1);
                } else if (cp1 < 0x800) {
                   onChar((char)(0xC0 | (cp1 >> 6)));
                   onChar((char)(0x80 | (cp1 & 0x3F)));
                } else {
                   onChar((char)(0xE0 | (cp1 >> 12)));
                   onChar((char)(0x80 | ((cp1 >> 6) & 0x3F)));
                   onChar((char)(0x80 | (cp1 & 0x3F)));
                }
                break;
              }
              default: onChar(next); break;
            }
        } else {
          onChar(ch);
        }
      }
    } else if (isdigit(c) || c == '-') {
      while (_stream.available()) {
        char current_char = _stream.peek();
        if (isdigit(current_char) || current_char == '.' || current_char == '-' || current_char == '+' || current_char == 'e' || current_char == 'E') {
          onChar(_read());
        } else {
          break;
        }
      }
    } else if (c == '{' || c == '[') {
      int balance = 0;
      do {
        while (!_stream.available()) delay(1);
        char ch = _read();
        onChar(ch);
        if (ch == '{' || ch == '[') balance++;
        else if (ch == '}' || ch == ']') balance--;
      } while (balance > 0);
    } else {
      _skipValue();
    }
  }

private:
  Stream &_stream;
  char _peek;
  char _read() {
    if (_peek != '\0') {
      char p = _peek;
      _peek = '\0';
      return p;
    }
    if (_stream.available()) {
      return _stream.read();
    }
    return '\0';
  }

  void _skipWhitespace() {
    while (_stream.available()) {
      int c = _stream.peek();
      if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
        _stream.read();
      } else {
        break;
      }
    }
  }

  void _skipString() {
      _read();
      while(_stream.available()){
          char c = _read();
          if(c == '"') break;
          if(c == '\\') _read();
      }
  }

  void _skipValue() {
    _skipWhitespace();
    char c = _stream.peek();
    if (c == '"') {
      _skipString();
    } else if (c == '{') {
      _skipObject();
    } else if (c == '[') {
      _skipArray();
    } else if (isdigit(c) || c == '-' || c == 't' || c == 'f' || c == 'n') {
      while (_stream.available()) {
        char current_char = _stream.peek();
        if (strchr(" \t\n\r,}]", current_char) == NULL) {
            _read();
        } else {
            break;
        }
      }
    }
  }

  void _skipObject() {
    _read();
    int balance = 1;
    while (_stream.available() && balance > 0) {
      char c = _read();
      if (c == '{') balance++;
      else if (c == '}') balance--;
      else if (c == '"') _skipString();
    }
  }

  void _skipArray() {
    _read();
    int balance = 1;
    while (_stream.available() && balance > 0) {
      char c = _read();
      if (c == '[') balance++;
      else if (c == ']') balance--;
      else if (c == '"') _skipString();
    }
  }

  String _parseString() {
    String result;
    if (!result.reserve(32)) {
      Serial.println(F("Error: Failed to reserve 32b for result in '_parseString()'!"));
    }
    while (_stream.available()) {
      char c = _read();
      if (c == '"') break;
      result += c;
    }
    return result;
  }

  bool _findKeyRecursive(const char *key_to_find) {
    _skipWhitespace();
    char current_char = _stream.peek();
    if (current_char == '{') {
      _read();
      while (true) {
        _skipWhitespace();
        current_char = _stream.peek();
        if (current_char == '}') {
          _read();
          return false;
        }
        if (current_char == ',') {
          _read();
          _skipWhitespace();
        }
        if (_stream.peek() != '"') return false;
        _read();
        String currentKey = _parseString();
        _skipWhitespace();
        if (_read() != ':') return false;
        if (strcmp(currentKey.c_str(), key_to_find) == 0) {
          return true;
        }
        _skipWhitespace();
        current_char = _stream.peek();
        if (current_char == '{' || current_char == '[') {
          if (_findKeyRecursive(key_to_find)) {
            return true;
          }
        } else {
          _skipValue();
        }
      }
    } else if (current_char == '[') {
      _read();
      while (true) {
        _skipWhitespace();
        current_char = _stream.peek();
        if (current_char == ']') {
          _read();
          return false;
        }
        if (current_char == ',') {
          _read();
        }
        if (_findKeyRecursive(key_to_find)) {
          return true;
        } else {
          _skipValue();
        }
      }
    }
    _skipValue();
    return false;
  }
};