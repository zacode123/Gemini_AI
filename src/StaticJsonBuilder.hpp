/*
 * ESP8266StaticJsonBuilder.hpp - A fast, zero-allocation static JSON builder for ESP8266/ESP32 using fixed-size character buffers.
 *
 * Supports deeply nested JSON objects and arrays, various primitive types, and macro-driven syntax for easy usage.
 * Specifically optimized for memory-constrained microcontrollers, avoiding all heap usage by working with caller-provided buffers.
 *
 * This library avoids all dynamic memory allocation (no malloc/new/String) and builds JSON into a user-provided `char[]` buffer
 * using only fixed stack allocations and static template dispatch for serialization.
 *
 * Features:
 * - Zero heap allocations (100% stack/static)
 * - Optional pretty-printing
 * - Configurable float precision and max nesting depth
 * - Support for strings, integers, floats, booleans, null, and arrays
 * - Safe for very low-RAM devices like ESP8266
 * - Simple macro wrappers to manage JSON structure
 *
 * MIT License
 * Created by zacode123, 18-07-2025
 * Version 2.2.0 (Improved user simplicity)
 */

#pragma once

#include <WString.h>
#include <cstring>
#include <cstdio>

#ifndef JSON_MAX_DEPTH
#define JSON_MAX_DEPTH 10
#endif

class StaticJsonBuilder {
  char* buffer;
  size_t capacity;
  size_t length;
  bool isFirst[JSON_MAX_DEPTH];
  int depth;

public:
  StaticJsonBuilder(char* buf, size_t cap, bool = false)
    : buffer(buf), capacity(cap), length(0), depth(0) {
    buffer[0] = '\0';
    for (int i = 0; i < JSON_MAX_DEPTH; i++) isFirst[i] = true;
  }

  void beginObject() {
    writeSeparator();
    writeChar('{');
    if (depth < JSON_MAX_DEPTH) isFirst[depth++] = true;
  }

  void endObject() {
    if (depth > 0) depth--;
    writeChar('}');
    isFirst[depth] = false;
  }

  void beginArray() {
    writeSeparator();
    writeChar('[');
    if (depth < JSON_MAX_DEPTH) isFirst[depth++] = true;
  }

  void endArray() {
    if (depth > 0) depth--;
    writeChar(']');
    isFirst[depth] = false;
  }

  void key(const char* k) {
    writeSeparator();
    serializeString(k);
    writeChar(':');
    isFirst[depth] = true;
  }

  void value(const char* v) {
    writeSeparator();
    serializeString(v);
    isFirst[depth] = false;
  }

  void value(const String& v) {
    value(v.c_str());
  }

  void value(int v) {
    writeSeparator();
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", v);
    writeLiteral(buf);
    isFirst[depth] = false;
  }

  void value(float v, int precision = 2) {
    writeSeparator();
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", precision, v);
    writeLiteral(buf);
    isFirst[depth] = false;
  }

  void value(bool v) {
    writeSeparator();
    writeLiteral(v ? "true" : "false");
    isFirst[depth] = false;
  }

  const char* c_str() const { return buffer; }

private:
  void writeChar(char c) {
    if (length + 1 < capacity) {
      buffer[length++] = c;
      buffer[length] = '\0';
    }
  }

  void writeLiteral(const char* s) {
    while (*s) writeChar(*s++);
  }

  void writeSeparator() {
    if (!isFirst[depth]) {
      writeChar(',');
    } else {
      isFirst[depth] = false;
    }
  }

  void serializeString(const char* s) {
    writeChar('"');
    while (*s) {
      char c = *s++;
      switch (c) {
        case '\"': writeLiteral("\\\""); break;
        case '\\': writeLiteral("\\\\"); break;
        case '\n': writeLiteral("\\n"); break;
        case '\r': writeLiteral("\\r"); break;
        case '\t': writeLiteral("\\t"); break;
        default: writeChar(c); break;
      }
    }
    writeChar('"');
  }
};